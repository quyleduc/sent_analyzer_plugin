#include "SENTAnalyzer.h"
#include "SENTAnalyzerSettings.h"
#include <AnalyzerChannelData.h>
#include <cmath>
#include <vector>

static const U8 s_crc4_table[16] = {0, 13, 7, 10, 14, 3, 9, 4, 1, 12, 6, 11, 15, 2, 8, 5};

SENTAnalyzer::SENTAnalyzer()
:   Analyzer2(),
    mSettings( new SENTAnalyzerSettings() ),
    mSimulationInitialized( false ),
    mSerial( nullptr ),
    mSamplesPerTick( 0.0 )
{
    SetAnalyzerSettings( mSettings.get() );
}

SENTAnalyzer::~SENTAnalyzer()
{
    KillThread();
}

void SENTAnalyzer::SetupResults()
{
    mResults.reset( new SENTAnalyzerResults( this, mSettings.get() ) );
    SetAnalyzerResults( mResults.get() );
    mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );
}

U8 SENTAnalyzer::CalculateCRC( U8 status_val, const std::vector<U8>& data_nibbles )
{
    if( mSettings->mLegacyCrc )
    {
        /* Legacy CRC: seed=5, 6 data nibbles */
        U8 crc = 5;
        for( size_t i = 0; i < data_nibbles.size(); i++ )
        {
            crc = s_crc4_table[crc ^ (data_nibbles[i] & 0xF)];
        }
        return crc & 0xF;
    }
    else
    {
        /* APR2016 CRC: seed=3, 7 nibbles (Status + 6 data nibbles) */
        U8 crc = 3;
        crc = s_crc4_table[crc ^ (status_val & 0xF)];
        for( size_t i = 0; i < data_nibbles.size(); i++ )
        {
            crc = s_crc4_table[crc ^ (data_nibbles[i] & 0xF)];
        }
        return crc & 0xF;
    }
}

void SENTAnalyzer::WorkerThread()
{
    U32 sample_rate = GetSampleRate();
    double tick_time_sec = (mSettings->mTickTimeHalfUs * 0.5) * 1e-6;
    mSamplesPerTick = sample_rate * tick_time_sec;
    if( mSamplesPerTick < 1.0 ) mSamplesPerTick = 1.0;

    mSerial = GetAnalyzerChannelData( mSettings->mInputChannel );

    /* Advance to first falling edge */
    if( mSerial->GetBitState() == BIT_LOW )
    {
        mSerial->AdvanceToNextEdge(); /* to rising */
    }
    mSerial->AdvanceToNextEdge(); /* to falling */

    std::vector<Frame> current_packet_frames;
    U8 status_val = 0;
    std::vector<U8> data_nibbles;
    U8 received_crc = 0;
    U16 pause_ticks = 0;
    U32 expected_data_nibbles = mSettings->mNumberOfDataNibbles;
    bool has_pause_pulse = mSettings->mPausePulseEnabled;

    U8 pulse_in_frame_idx = 0; /* 0=Sync, 1=Status, 2..7=Data, 8=CRC, 9=Pause */

    for( ; ; )
    {
        U64 start_sample = mSerial->GetSampleNumber();

        /* Advance 2 edges to the next falling edge */
        mSerial->AdvanceToNextEdge(); /* to rising */
        mSerial->AdvanceToNextEdge(); /* to falling */

        U64 end_sample = mSerial->GetSampleNumber();
        U64 num_samples = end_sample - start_sample;

        U32 ticks = (U32)std::round( (double)num_samples / mSamplesPerTick );

        /* Check if this is a SYNC pulse (56 ticks +/- 20% -> 45..67 ticks) */
        bool is_sync = ( ticks >= 45 && ticks <= 67 );
        if( has_pause_pulse && pulse_in_frame_idx == (1 + expected_data_nibbles + 1) )
        {
            /* If we are at pause pulse position, and pause happens to be ~56 ticks, treat as pause */
            is_sync = false;
        }

        if( is_sync )
        {
            /* Calibrate tick time dynamically based on the 56-tick Sync pulse */
            mSamplesPerTick = (double)num_samples / 56.0;

            /* Check if previous frame was complete */
            U32 needed_frames = 1 + 1 + expected_data_nibbles + 1; /* Sync + Status + Data + CRC */
            if( has_pause_pulse ) needed_frames += 1;

            if( current_packet_frames.size() == needed_frames && data_nibbles.size() == expected_data_nibbles )
            {
                U8 expected_crc = CalculateCRC( status_val, data_nibbles );
                bool crc_ok = ( received_crc == expected_crc );

                for( size_t fi = 0; fi < current_packet_frames.size(); fi++ )
                {
                    Frame f = current_packet_frames[fi];
                    if( f.mType == CRCNibble && !crc_ok )
                    {
                        f.mFlags |= (1 << CrcError);
                        f.mData2 = expected_crc; /* Store expected CRC */
                    }
                    mResults->AddFrame( f );
                    mResults->CommitResults();
                    ReportProgress( f.mEndingSampleInclusive );
                }
                mResults->CommitPacketAndStartNewPacket();
            }
            else if( current_packet_frames.size() > 0 )
            {
                /* Incomplete frame at beginning of capture */
                Frame err;
                err.mStartingSampleInclusive = current_packet_frames.front().mStartingSampleInclusive;
                err.mEndingSampleInclusive = current_packet_frames.back().mEndingSampleInclusive;
                err.mType = ErrorPulse;
                err.mFlags = DISPLAY_AS_ERROR_FLAG | (1 << NibbleNumberError);
                err.mData1 = (U64)current_packet_frames.size();
                err.mData2 = 0;
                mResults->AddFrame( err );
                mResults->CommitResults();
                ReportProgress( err.mEndingSampleInclusive );
                mResults->CommitPacketAndStartNewPacket();
            }

            /* Start new packet */
            current_packet_frames.clear();
            data_nibbles.clear();
            status_val = 0;
            received_crc = 0;
            pause_ticks = 0;

            Frame sync_frame;
            sync_frame.mStartingSampleInclusive = start_sample;
            sync_frame.mEndingSampleInclusive = end_sample;
            sync_frame.mType = SyncPulse;
            sync_frame.mFlags = 0;
            sync_frame.mData1 = 56; /* 56 Ticks */
            sync_frame.mData2 = 0;
            current_packet_frames.push_back( sync_frame );

            pulse_in_frame_idx = 1; /* Expect Status next */
        }
        else if( pulse_in_frame_idx == 1 )
        {
            /* Status Nibble (12..27 ticks -> 0..15) */
            U8 val = ( ticks >= 12 ) ? (U8)(ticks - 12) : 0;
            if( val > 15 ) val = 15;
            status_val = val;

            Frame sf;
            sf.mStartingSampleInclusive = start_sample;
            sf.mEndingSampleInclusive = end_sample;
            sf.mType = StatusNibble;
            sf.mFlags = 0;
            sf.mData1 = val;
            sf.mData2 = 0;
            current_packet_frames.push_back( sf );

            pulse_in_frame_idx = 2; /* Expect Data D0 next */
        }
        else if( pulse_in_frame_idx >= 2 && pulse_in_frame_idx < (2 + expected_data_nibbles) )
        {
            /* Fast Channel Data Nibble (12..27 ticks -> 0..15) */
            U8 nib_idx = pulse_in_frame_idx - 2;
            U8 val = ( ticks >= 12 ) ? (U8)(ticks - 12) : 0;
            if( val > 15 ) val = 15;
            data_nibbles.push_back( val );

            Frame df;
            df.mStartingSampleInclusive = start_sample;
            df.mEndingSampleInclusive = end_sample;
            df.mType = DataNibble;
            df.mFlags = 0;
            df.mData1 = val;
            df.mData2 = nib_idx; /* store nibble index 0..5 */
            current_packet_frames.push_back( df );

            pulse_in_frame_idx++;
        }
        else if( pulse_in_frame_idx == (2 + expected_data_nibbles) )
        {
            /* CRC Nibble (12..27 ticks -> 0..15) */
            U8 val = ( ticks >= 12 ) ? (U8)(ticks - 12) : 0;
            if( val > 15 ) val = 15;
            received_crc = val;

            Frame cf;
            cf.mStartingSampleInclusive = start_sample;
            cf.mEndingSampleInclusive = end_sample;
            cf.mType = CRCNibble;
            cf.mFlags = 0;
            cf.mData1 = val;
            cf.mData2 = 0;
            current_packet_frames.push_back( cf );

            if( has_pause_pulse )
            {
                pulse_in_frame_idx++; /* Expect Pause pulse */
            }
            else
            {
                pulse_in_frame_idx = 0; /* Expect Sync pulse */
            }
        }
        else if( has_pause_pulse && pulse_in_frame_idx == (2 + expected_data_nibbles + 1) )
        {
            /* Pause Pulse (>= 12 ticks) */
            pause_ticks = (U16)ticks;

            Frame pf;
            pf.mStartingSampleInclusive = start_sample;
            pf.mEndingSampleInclusive = end_sample;
            pf.mType = PausePulse;
            pf.mFlags = 0;
            pf.mData1 = ticks; /* actual pause ticks */
            pf.mData2 = 0;
            current_packet_frames.push_back( pf );

            pulse_in_frame_idx = 0; /* Next pulse MUST be Sync */
        }
        else
        {
            /* Unexpected pulse -> reset index */
            pulse_in_frame_idx = 0;
        }

        CheckIfThreadShouldExit();
    }
}

bool SENTAnalyzer::NeedsRerun()
{
    return false;
}

U32 SENTAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
    if( !mSimulationInitialized )
    {
        mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
        mSimulationInitialized = true;
    }
    return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 SENTAnalyzer::GetMinimumSampleRateHz()
{
    return 2000000;
}

const char* SENTAnalyzer::GetAnalyzerName() const
{
    return "SENT (SAE J2716)";
}

const char* GetAnalyzerName()
{
    return "SENT (SAE J2716)";
}

Analyzer* CreateAnalyzer()
{
    return new SENTAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
    delete analyzer;
}