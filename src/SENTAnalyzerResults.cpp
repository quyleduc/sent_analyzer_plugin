#include "SENTAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "SENTAnalyzer.h"
#include "SENTAnalyzerSettings.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

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

    char val_str[64];
    AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, val_str, sizeof(val_str) );

    switch( (SENTNibbleType)frame.mType )
    {
        case SyncPulse:
            AddResultString( "SYNC (56T)" );
            AddResultString( "SYNC" );
            AddResultString( "S" );
            break;

        case StatusNibble:
            {
                char buf1[64], buf2[32];
                snprintf( buf1, sizeof(buf1), "Status: %s", val_str );
                snprintf( buf2, sizeof(buf2), "St: %s", val_str );
                AddResultString( buf1 );
                AddResultString( buf2 );
                AddResultString( val_str );
            }
            break;

        case DataNibble:
            {
                /* frame.mData2 stores the nibble index (0..5) */
                U8 nib_idx = (U8)frame.mData2;
                char buf1[64], buf2[32];
                snprintf( buf1, sizeof(buf1), "Data D%u: %s", nib_idx, val_str );
                snprintf( buf2, sizeof(buf2), "D%u: %s", nib_idx, val_str );
                AddResultString( buf1 );
                AddResultString( buf2 );
                AddResultString( val_str );
            }
            break;

        case CRCNibble:
            {
                bool is_error = ( (frame.mFlags & (1 << CrcError)) != 0 );
                char buf1[64], buf2[32];
                if( is_error )
                {
                    snprintf( buf1, sizeof(buf1), "CRC ERR: %s", val_str );
                    snprintf( buf2, sizeof(buf2), "CRC: !%s", val_str );
                }
                else
                {
                    snprintf( buf1, sizeof(buf1), "CRC: %s (PASS)", val_str );
                    snprintf( buf2, sizeof(buf2), "CRC: %s", val_str );
                }
                AddResultString( buf1 );
                AddResultString( buf2 );
                AddResultString( val_str );
            }
            break;

        case PausePulse:
            {
                char buf1[64], buf2[32];
                snprintf( buf1, sizeof(buf1), "Pause: %s Ticks", val_str );
                snprintf( buf2, sizeof(buf2), "Pause: %sT", val_str );
                AddResultString( buf1 );
                AddResultString( buf2 );
                AddResultString( "P" );
            }
            break;

        case ErrorPulse:
        default:
            {
                if( (frame.mFlags & (1 << NibbleNumberError)) != 0 )
                {
                    char buf[64];
                    snprintf( buf, sizeof(buf), "Error: %s nibbles detected", val_str );
                    AddResultString( buf );
                    AddResultString( "ERR: Count" );
                    AddResultString( "ERR" );
                }
                else if( (frame.mFlags & (1 << CrcError)) != 0 )
                {
                    char buf[64];
                    snprintf( buf, sizeof(buf), "Error: CRC mismatch (expected %s)", val_str );
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

    char val_str[64];
    AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, val_str, sizeof(val_str) );

    char text[128];
    switch( (SENTNibbleType)frame.mType )
    {
        case SyncPulse:
            snprintf( text, sizeof(text), "SYNC (56 Ticks)" );
            break;
        case StatusNibble:
            snprintf( text, sizeof(text), "Status: %s", val_str );
            break;
        case DataNibble:
            snprintf( text, sizeof(text), "Data D%u: %s", (U32)frame.mData2, val_str );
            break;
        case CRCNibble:
            snprintf( text, sizeof(text), "CRC: %s%s", val_str, (frame.mFlags & (1 << CrcError)) ? " (CRC ERROR)" : " (OK)" );
            break;
        case PausePulse:
            snprintf( text, sizeof(text), "Pause: %s Ticks", val_str );
            break;
        case ErrorPulse:
        default:
            snprintf( text, sizeof(text), "Error (%s)", val_str );
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
    U8 d[6] = {0};
    U8 d_count = 0;
    U8 crc = 0;
    U16 pause_ticks = 0;
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
            if( d_count < 6 )
            {
                d[d_count++] = (U8)(f.mData1 & 0xF);
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

    /* Fast Data 1 (12-bit) and Fast Data 2 (12-bit) */
    U16 fast1 = ( (U16)d[0] << 8 ) | ( (U16)d[1] << 4 ) | (U16)d[2];
    U16 fast2 = ( (U16)d[3] << 8 ) | ( (U16)d[4] << 4 ) | (U16)d[5];

    char packet_str[256];
    snprintf( packet_str, sizeof(packet_str),
              "SENT | Status: 0x%X | Fast1: %u (0x%03X) | Fast2: %u (0x%03X) | CRC: 0x%X [%s] | Pause: %uT",
              status, fast1, fast1, fast2, fast2, crc, crc_ok ? "PASS" : "FAIL", pause_ticks );

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
    file_stream << "Packet ID,Time [s],Status,FastData1,FastData2,CRC,CRC_Status,Pause_Ticks" << std::endl;

    for( U32 i = 0; i < number_of_packets; i++ )
    {
        U64 first_fid, last_fid;
        GetFramesContainedInPacket( i, &first_fid, &last_fid );

        if( first_fid > last_fid ) continue;

        Frame first_f = GetFrame( first_fid );
        char time_str[128];
        AnalyzerHelpers::GetTimeString( first_f.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, sizeof(time_str) );

        U8 status = 0;
        U8 d[6] = {0};
        U8 d_count = 0;
        U8 crc = 0;
        U16 pause = 0;
        bool crc_ok = true;

        for( U64 fid = first_fid; fid <= last_fid; fid++ )
        {
            Frame f = GetFrame( fid );
            if( f.mType == StatusNibble ) status = (U8)(f.mData1 & 0xF);
            else if( f.mType == DataNibble && d_count < 6 ) d[d_count++] = (U8)(f.mData1 & 0xF);
            else if( f.mType == CRCNibble ) {
                crc = (U8)(f.mData1 & 0xF);
                if( (f.mFlags & (1 << CrcError)) != 0 ) crc_ok = false;
            }
            else if( f.mType == PausePulse ) pause = (U16)f.mData1;
        }

        U16 fast1 = ( (U16)d[0] << 8 ) | ( (U16)d[1] << 4 ) | (U16)d[2];
        U16 fast2 = ( (U16)d[3] << 8 ) | ( (U16)d[4] << 4 ) | (U16)d[5];

        file_stream << i << "," << time_str << ",0x" << std::hex << (int)status << std::dec
                    << "," << fast1 << "," << fast2 << ",0x" << std::hex << (int)crc << std::dec
                    << "," << (crc_ok ? "PASS" : "FAIL") << "," << pause << std::endl;

        if( UpdateExportProgressAndCheckForCancel( i, number_of_packets ) == true )
        {
            file_stream.close();
            return;
        }
    }

    file_stream.close();
}