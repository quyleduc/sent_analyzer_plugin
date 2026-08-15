#include "SENTAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "SENTAnalyzer.h"
#include "SENTAnalyzerSettings.h"
#include "SENTProfiles.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <vector>

SENTAnalyzerResults::SENTAnalyzerResults( SENTAnalyzer* analyzer, SENTAnalyzerSettings* settings )
:   AnalyzerResults(),
    mSettings( settings ),
    mAnalyzer( analyzer )
{
}

SENTAnalyzerResults::~SENTAnalyzerResults()
{
}

void SENTAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
    ClearResultStrings();
    Frame frame = GetFrame( frame_index );

    char val_hex[32];
    AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, val_hex, sizeof(val_hex) );

    switch( (SENTNibbleType)frame.mType )
    {
        case SyncPulse:
            AddResultString( "SYNC (56 Ticks)" );
            AddResultString( "SYNC (56T)" );
            AddResultString( "SYNC" );
            AddResultString( "56T" );
            AddResultString( "S" );
            break;

        case StatusNibble:
            {
                char b1[64], b2[32], b3[16];
                snprintf( b1, sizeof(b1), "Status: %s", val_hex );
                snprintf( b2, sizeof(b2), "St: %s", val_hex );
                snprintf( b3, sizeof(b3), "St:%u", (U32)frame.mData1 );
                AddResultString( b1 );
                AddResultString( b2 );
                AddResultString( b3 );
            }
            break;

        case DataNibble:
            {
                U8 nib_idx = (U8)(frame.mData2 & 0xF);
                U16 full_raw = (U16)(frame.mData2 >> 8);

                if( mSettings->mProfileType == PROFILE_TMAP_A2 )
                {
                    if( nib_idx == 0 )
                    {
                        double p_kpa = 20.0 + ( (double)full_raw / 4095.0 ) * 280.0;
                        char b1[64], b2[32], b3[16];
                        snprintf( b1, sizeof(b1), "D0: %s (P: %.1f kPa)", val_hex, p_kpa );
                        snprintf( b2, sizeof(b2), "P: %.1f kPa", p_kpa );
                        snprintf( b3, sizeof(b3), "D0:%s", val_hex );
                        AddResultString( b1 );
                        AddResultString( b2 );
                        AddResultString( b3 );
                        break;
                    }
                    else if( nib_idx == 3 )
                    {
                        double t_c = -40.0 + ( (double)full_raw / 4095.0 ) * 190.0;
                        char b1[64], b2[32], b3[16];
                        snprintf( b1, sizeof(b1), "D3: %s (T: %.1f °C)", val_hex, t_c );
                        snprintf( b2, sizeof(b2), "T: %.1f °C", t_c );
                        snprintf( b3, sizeof(b3), "D3:%s", val_hex );
                        AddResultString( b1 );
                        AddResultString( b2 );
                        AddResultString( b3 );
                        break;
                    }
                }
                else if( mSettings->mProfileType == PROFILE_DUAL_THROTTLE_A1 )
                {
                    if( nib_idx == 0 )
                    {
                        double pct = ( (double)full_raw / 4095.0 ) * 100.0;
                        char b1[64], b2[32], b3[16];
                        snprintf( b1, sizeof(b1), "D0: %s (TPS1: %.1f%%)", val_hex, pct );
                        snprintf( b2, sizeof(b2), "TPS1: %.1f%%", pct );
                        snprintf( b3, sizeof(b3), "D0:%s", val_hex );
                        AddResultString( b1 );
                        AddResultString( b2 );
                        AddResultString( b3 );
                        break;
                    }
                    else if( nib_idx == 3 )
                    {
                        double pct = ( (double)full_raw / 4095.0 ) * 100.0;
                        char b1[64], b2[32], b3[16];
                        snprintf( b1, sizeof(b1), "D3: %s (TPS2: %.1f%%)", val_hex, pct );
                        snprintf( b2, sizeof(b2), "TPS2: %.1f%%", pct );
                        snprintf( b3, sizeof(b3), "D3:%s", val_hex );
                        AddResultString( b1 );
                        AddResultString( b2 );
                        AddResultString( b3 );
                        break;
                    }
                }
                else if( mSettings->mProfileType == PROFILE_MAF_A3 )
                {
                    if( nib_idx == 0 )
                    {
                        double maf = ( (double)full_raw / 16383.0 ) * 640.0;
                        char b1[64], b2[32], b3[16];
                        snprintf( b1, sizeof(b1), "D0: %s (MAF: %.1f kg/h)", val_hex, maf );
                        snprintf( b2, sizeof(b2), "MAF: %.1f kg/h", maf );
                        snprintf( b3, sizeof(b3), "D0:%s", val_hex );
                        AddResultString( b1 );
                        AddResultString( b2 );
                        AddResultString( b3 );
                        break;
                    }
                    else if( nib_idx == 3 )
                    {
                        double temp = -40.0 + ( (double)full_raw / 1023.0 ) * 160.0;
                        char b1[64], b2[32], b3[16];
                        snprintf( b1, sizeof(b1), "D3: %s (Temp: %.1f °C)", val_hex, temp );
                        snprintf( b2, sizeof(b2), "T: %.1f °C", temp );
                        snprintf( b3, sizeof(b3), "D3:%s", val_hex );
                        AddResultString( b1 );
                        AddResultString( b2 );
                        AddResultString( b3 );
                        break;
                    }
                }
                else if( mSettings->mProfileType == PROFILE_SECURE_A4 )
                {
                    if( nib_idx == 0 )
                    {
                        char b1[64], b2[32], b3[16];
                        snprintf( b1, sizeof(b1), "D0: %s (Sig: %u)", val_hex, full_raw );
                        snprintf( b2, sizeof(b2), "Sig: %u", full_raw );
                        snprintf( b3, sizeof(b3), "D0:%s", val_hex );
                        AddResultString( b1 );
                        AddResultString( b2 );
                        AddResultString( b3 );
                        break;
                    }
                    else if( nib_idx == 3 )
                    {
                        char b1[64], b2[32], b3[16];
                        snprintf( b1, sizeof(b1), "D3: %s (Cnt: %u)", val_hex, (U32)(full_raw & 0xFF) );
                        snprintf( b2, sizeof(b2), "Cnt: %u", (U32)(full_raw & 0xFF) );
                        snprintf( b3, sizeof(b3), "D3:%s", val_hex );
                        AddResultString( b1 );
                        AddResultString( b2 );
                        AddResultString( b3 );
                        break;
                    }
                }
                else if( mSettings->mProfileType == PROFILE_SINGLE_16_A5 )
                {
                    if( nib_idx == 0 )
                    {
                        char b1[64], b2[32], b3[16];
                        snprintf( b1, sizeof(b1), "D0: %s (Sig16: 0x%04X)", val_hex, full_raw );
                        snprintf( b2, sizeof(b2), "Sig: 0x%04X", full_raw );
                        snprintf( b3, sizeof(b3), "D0:%s", val_hex );
                        AddResultString( b1 );
                        AddResultString( b2 );
                        AddResultString( b3 );
                        break;
                    }
                }

                char b1[64], b2[32], b3[16];
                snprintf( b1, sizeof(b1), "Data D%u: %s", nib_idx, val_hex );
                snprintf( b2, sizeof(b2), "D%u: %s", nib_idx, val_hex );
                snprintf( b3, sizeof(b3), "D%u:%s", nib_idx, val_hex );
                AddResultString( b1 );
                AddResultString( b2 );
                AddResultString( b3 );
            }
            break;

        case CRCNibble:
            {
                bool is_error = ( (frame.mFlags & (1 << CrcError)) != 0 );
                char b1[64], b2[32], b3[16];
                if( is_error )
                {
                    snprintf( b1, sizeof(b1), "CRC ERR: %s", val_hex );
                    snprintf( b2, sizeof(b2), "CRC: !%s", val_hex );
                    snprintf( b3, sizeof(b3), "!CRC" );
                }
                else
                {
                    snprintf( b1, sizeof(b1), "CRC: %s (PASS)", val_hex );
                    snprintf( b2, sizeof(b2), "CRC: %s", val_hex );
                    snprintf( b3, sizeof(b3), "CRC" );
                }
                AddResultString( b1 );
                AddResultString( b2 );
                AddResultString( b3 );
            }
            break;

        case PausePulse:
            {
                U32 p_ticks = (U32)frame.mData1;
                char b1[64], b2[32], b3[16];
                snprintf( b1, sizeof(b1), "Pause: %u Ticks", p_ticks );
                snprintf( b2, sizeof(b2), "Pause: %uT", p_ticks );
                snprintf( b3, sizeof(b3), "P:%uT", p_ticks );
                AddResultString( b1 );
                AddResultString( b2 );
                AddResultString( b3 );
                AddResultString( "P" );
            }
            break;

        case ErrorPulse:
        default:
            {
                if( (frame.mFlags & (1 << NibbleNumberError)) != 0 )
                {
                    char buf[64];
                    snprintf( buf, sizeof(buf), "Error: %s nibbles detected", val_hex );
                    AddResultString( buf );
                    AddResultString( "ERR: Count" );
                    AddResultString( "ERR" );
                }
                else if( (frame.mFlags & (1 << CrcError)) != 0 )
                {
                    char buf[64];
                    snprintf( buf, sizeof(buf), "Error: CRC mismatch (expected %s)", val_hex );
                    AddResultString( buf );
                    AddResultString( "ERR: CRC" );
                    AddResultString( "ERR" );
                }
                else
                {
                    AddResultString( "Error" );
                    AddResultString( "ERR" );
                }
            }
            break;
    }
}

void SENTAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
    ClearTabularText();
    Frame frame = GetFrame( frame_index );

    char val_hex[32];
    AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, val_hex, sizeof(val_hex) );

    char text[256];
    switch( (SENTNibbleType)frame.mType )
    {
        case SyncPulse:
            snprintf( text, sizeof(text), "SYNC (56 Ticks)" );
            break;
        case StatusNibble:
            snprintf( text, sizeof(text), "Status: %s", val_hex );
            break;
        case DataNibble:
            {
                U8 nib_idx = (U8)(frame.mData2 & 0xF);
                U16 full_raw = (U16)(frame.mData2 >> 8);
                if( mSettings->mProfileType == PROFILE_TMAP_A2 )
                {
                    if( nib_idx == 0 )
                    {
                        double p_kpa = 20.0 + ( (double)full_raw / 4095.0 ) * 280.0;
                        snprintf( text, sizeof(text), "Data D0: %s -> Pressure: %.1f kPa (0x%03X)", val_hex, p_kpa, full_raw );
                        break;
                    }
                    else if( nib_idx == 3 )
                    {
                        double t_c = -40.0 + ( (double)full_raw / 4095.0 ) * 190.0;
                        snprintf( text, sizeof(text), "Data D3: %s -> Temp: %.1f °C (0x%03X)", val_hex, t_c, full_raw );
                        break;
                    }
                }
                else if( mSettings->mProfileType == PROFILE_DUAL_THROTTLE_A1 )
                {
                    if( nib_idx == 0 )
                    {
                        double pct = ( (double)full_raw / 4095.0 ) * 100.0;
                        snprintf( text, sizeof(text), "Data D0: %s -> TPS1: %.1f%% (0x%03X)", val_hex, pct, full_raw );
                        break;
                    }
                    else if( nib_idx == 3 )
                    {
                        double pct = ( (double)full_raw / 4095.0 ) * 100.0;
                        snprintf( text, sizeof(text), "Data D3: %s -> TPS2: %.1f%% (0x%03X)", val_hex, pct, full_raw );
                        break;
                    }
                }
                else if( mSettings->mProfileType == PROFILE_MAF_A3 )
                {
                    if( nib_idx == 0 )
                    {
                        double maf = ( (double)full_raw / 16383.0 ) * 640.0;
                        snprintf( text, sizeof(text), "Data D0: %s -> MAF: %.1f kg/h", val_hex, maf );
                        break;
                    }
                    else if( nib_idx == 3 )
                    {
                        double temp = -40.0 + ( (double)full_raw / 1023.0 ) * 160.0;
                        snprintf( text, sizeof(text), "Data D3: %s -> Temp: %.1f °C", val_hex, temp );
                        break;
                    }
                }
                else if( mSettings->mProfileType == PROFILE_SECURE_A4 )
                {
                    if( nib_idx == 0 )
                    {
                        snprintf( text, sizeof(text), "Data D0: %s -> Signal: %u", val_hex, full_raw );
                        break;
                    }
                    else if( nib_idx == 3 )
                    {
                        snprintf( text, sizeof(text), "Data D3: %s -> Rolling Counter: %u", val_hex, (U32)(full_raw & 0xFF) );
                        break;
                    }
                }
                snprintf( text, sizeof(text), "Data D%u: %s", (U32)nib_idx, val_hex );
            }
            break;
        case CRCNibble:
            snprintf( text, sizeof(text), "CRC: %s%s", val_hex, (frame.mFlags & (1 << CrcError)) ? " (CRC ERROR)" : " (PASS)" );
            break;
        case PausePulse:
            snprintf( text, sizeof(text), "Pause: %u Ticks (%s)", (U32)frame.mData1, val_hex );
            break;
        case ErrorPulse:
        default:
            snprintf( text, sizeof(text), "Error (%s)", val_hex );
            break;
    }

    AddTabularText( text );
}

void SENTAnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
    ClearTabularText();

    U64 first_frame_id, last_frame_id;
    GetFramesContainedInPacket( packet_id, &first_frame_id, &last_frame_id );

    U8 status = 0;
    std::vector<U8> d;
    U8 crc = 0;
    U16 pause_ticks = 0;
    bool has_pause = false;
    bool crc_ok = true;
    bool has_error = false;

    for( U64 fid = first_frame_id; fid <= last_frame_id; fid++ )
    {
        Frame f = GetFrame( fid );
        if( f.mType == StatusNibble )
        {
            status = (U8)(f.mData1 & 0xF);
        }
        else if( f.mType == DataNibble )
        {
            if( d.size() < 6 )
            {
                d.push_back( (U8)(f.mData1 & 0xF) );
            }
        }
        else if( f.mType == CRCNibble )
        {
            crc = (U8)(f.mData1 & 0xF);
            if( (f.mFlags & (1 << CrcError)) != 0 )
            {
                crc_ok = false;
            }
        }
        else if( f.mType == PausePulse )
        {
            pause_ticks = (U16)f.mData1;
            has_pause = true;
        }
        else if( f.mType == ErrorPulse )
        {
            has_error = true;
        }
    }

    if( has_error )
    {
        AddTabularText( "SENT FRAME ERROR" );
        return;
    }

    /* Decode according to configured profile */
    DecodedProfileResult pres = SENTProfiles::DecodeFrame( mSettings->mProfileType, d, status );

    char packet_str[256];
    if( has_pause )
    {
        snprintf( packet_str, sizeof(packet_str),
                  "SENT | %s | Status: 0x%X | CRC: 0x%X [%s] | Pause: %uT",
                  pres.tabular_summary.c_str(), status, crc, crc_ok ? "PASS" : "FAIL", pause_ticks );
    }
    else
    {
        snprintf( packet_str, sizeof(packet_str),
                  "SENT | %s | Status: 0x%X | CRC: 0x%X [%s]",
                  pres.tabular_summary.c_str(), status, crc, crc_ok ? "PASS" : "FAIL" );
    }

    AddTabularText( packet_str );
}

void SENTAnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
    // not used
}

void SENTAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
    U32 number_of_packets = GetNumPackets();
    U64 trigger_sample = mAnalyzer->GetTriggerSample();
    U32 sample_rate = mAnalyzer->GetSampleRate();

    std::ofstream file_stream( file, std::ios::out );
    file_stream << "Packet ID,Time [s],Profile,Decoded Summary,Status,CRC,CRC_Status,Pause_Ticks" << std::endl;

    for( U32 i = 0; i < number_of_packets; i++ )
    {
        U64 first_fid, last_fid;
        GetFramesContainedInPacket( i, &first_fid, &last_fid );

        if( first_fid > last_fid ) continue;

        Frame first_f = GetFrame( first_fid );
        char time_str[128];
        AnalyzerHelpers::GetTimeString( first_f.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, sizeof(time_str) );

        U8 status = 0;
        std::vector<U8> d;
        U8 crc = 0;
        U16 pause = 0;
        bool crc_ok = true;

        for( U64 fid = first_fid; fid <= last_fid; fid++ )
        {
            Frame f = GetFrame( fid );
            if( f.mType == StatusNibble ) status = (U8)(f.mData1 & 0xF);
            else if( f.mType == DataNibble && d.size() < 6 ) d.push_back( (U8)(f.mData1 & 0xF) );
            else if( f.mType == CRCNibble ) {
                crc = (U8)(f.mData1 & 0xF);
                if( (f.mFlags & (1 << CrcError)) != 0 ) crc_ok = false;
            }
            else if( f.mType == PausePulse ) pause = (U16)f.mData1;
        }

        DecodedProfileResult pres = SENTProfiles::DecodeFrame( mSettings->mProfileType, d, status );

        file_stream << i << "," << time_str << ",\"" << pres.profile_name << "\",\""
                    << pres.tabular_summary << "\",0x" << std::hex << (int)status << std::dec
                    << ",0x" << std::hex << (int)crc << std::dec << ","
                    << (crc_ok ? "PASS" : "FAIL") << "," << pause << std::endl;

        if( UpdateExportProgressAndCheckForCancel( i, number_of_packets ) == true )
        {
            file_stream.close();
            return;
        }
    }

    file_stream.close();
}