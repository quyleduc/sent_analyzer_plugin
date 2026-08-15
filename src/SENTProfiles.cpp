#include "SENTProfiles.h"
#include <cstdio>
#include <cmath>

const char* SENTProfiles::GetProfileName( SENTProfileType type )
{
    switch( type )
    {
        case PROFILE_DUAL_THROTTLE_A1: return "A.1: Dual Throttle (TPS %)";
        case PROFILE_TMAP_A2:          return "A.2: TMAP (Pressure & Temp)";
        case PROFILE_MAF_A3:           return "A.3: MAF (Flow 14b & Temp 10b)";
        case PROFILE_SECURE_A4:        return "A.4: Secure (12b + 8b Counter)";
        case PROFILE_SINGLE_16_A5:     return "A.5: Single 16-bit High Res";
        case PROFILE_RAW_12_12:
        default:                       return "Raw Fast Channels (12b + 12b)";
    }
}

DecodedProfileResult SENTProfiles::DecodeFrame( SENTProfileType type, const std::vector<U8>& d, U8 status_val )
{
    DecodedProfileResult res;
    res.profile_name = GetProfileName( type );
    res.has_safety_alert = false;

    if( d.size() < 6 )
    {
        res.tabular_summary = "Incomplete data";
        return res;
    }

    char buf[256];

    switch( type )
    {
        case PROFILE_DUAL_THROTTLE_A1:
            {
                /* Fast 1: D0..D2 (12-bit), Fast 2: D3..D5 (12-bit) */
                U16 raw1 = ( (U16)d[0] << 8 ) | ( (U16)d[1] << 4 ) | d[2];
                U16 raw2 = ( (U16)d[3] << 8 ) | ( (U16)d[4] << 4 ) | d[5];

                double pct1 = ( (double)raw1 / 4095.0 ) * 100.0;
                double pct2 = ( (double)raw2 / 4095.0 ) * 100.0;

                /* Dual redundant correlation check (Sum should be ~4095 or close) */
                int sum = (int)raw1 + (int)raw2;
                bool is_inverted_pair = ( std::abs( sum - 4095 ) <= 120 );

                char ch1_buf[64], ch2_buf[64];
                snprintf( ch1_buf, sizeof(ch1_buf), "TPS1: %.1f%%", pct1 );
                snprintf( ch2_buf, sizeof(ch2_buf), "TPS2: %.1f%%", pct2 );
                res.ch1_label = ch1_buf;
                res.ch2_label = ch2_buf;

                if( is_inverted_pair )
                {
                    snprintf( buf, sizeof(buf), "TPS1: %.1f%% | TPS2: %.1f%% [Redundant OK]", pct1, pct2 );
                }
                else
                {
                    snprintf( buf, sizeof(buf), "TPS1: %.1f%% | TPS2: %.1f%%", pct1, pct2 );
                }
                res.tabular_summary = buf;
            }
            break;

        case PROFILE_TMAP_A2:
            {
                /* Fast 1 (Pressure): D0..D2 (12-bit) -> 20.0 to 300.0 kPa */
                U16 raw_p = ( (U16)d[0] << 8 ) | ( (U16)d[1] << 4 ) | d[2];
                /* Fast 2 (Temperature): D3..D5 (12-bit) -> -40.0 to +150.0 °C */
                U16 raw_t = ( (U16)d[3] << 8 ) | ( (U16)d[4] << 4 ) | d[5];

                double press_kpa = 20.0 + ( (double)raw_p / 4095.0 ) * 280.0;
                double temp_c = -40.0 + ( (double)raw_t / 4095.0 ) * 190.0;

                char ch1_buf[64], ch2_buf[64];
                snprintf( ch1_buf, sizeof(ch1_buf), "P: %.1f kPa", press_kpa );
                snprintf( ch2_buf, sizeof(ch2_buf), "T: %.1f °C", temp_c );
                res.ch1_label = ch1_buf;
                res.ch2_label = ch2_buf;

                snprintf( buf, sizeof(buf), "Pressure: %.1f kPa | Temp: %.1f °C", press_kpa, temp_c );
                res.tabular_summary = buf;
            }
            break;

        case PROFILE_MAF_A3:
            {
                /* 14-bit Mass Air Flow + 10-bit Temperature */
                /* D0: bits 13..10, D1: bits 9..6, D2: bits 5..2, D3 upper: bits 1..0 */
                U16 raw_maf = ( (U16)d[0] << 10 ) | ( (U16)d[1] << 6 ) | ( (U16)d[2] << 2 ) | ( (d[3] >> 2) & 0x3 );
                /* D3 lower: bits 9..8, D4: bits 7..4, D5: bits 3..0 */
                U16 raw_t10 = ( ((U16)d[3] & 0x3) << 8 ) | ( (U16)d[4] << 4 ) | d[5];

                double maf_kgh = ( (double)raw_maf / 16383.0 ) * 640.0;
                double temp_c = -40.0 + ( (double)raw_t10 / 1023.0 ) * 160.0;

                char ch1_buf[64], ch2_buf[64];
                snprintf( ch1_buf, sizeof(ch1_buf), "MAF: %.1f kg/h", maf_kgh );
                snprintf( ch2_buf, sizeof(ch2_buf), "Temp: %.1f °C", temp_c );
                res.ch1_label = ch1_buf;
                res.ch2_label = ch2_buf;

                snprintf( buf, sizeof(buf), "Flow: %.1f kg/h (14b: %u) | Temp: %.1f °C (10b: %u)", maf_kgh, raw_maf, temp_c, raw_t10 );
                res.tabular_summary = buf;
            }
            break;

        case PROFILE_SECURE_A4:
            {
                /* 12-bit Data (D0..D2) + 8-bit Counter (D3..D4) + 4-bit Inverted (D5) */
                U16 raw_sig = ( (U16)d[0] << 8 ) | ( (U16)d[1] << 4 ) | d[2];
                U8 counter = ( d[3] << 4 ) | d[4];
                U8 inv_check = d[5];

                bool inv_ok = ( (d[0] ^ inv_check) == 0xF );
                if( !inv_ok )
                {
                    res.has_safety_alert = true;
                    res.alert_msg = "INV_NIBBLE_MISMATCH";
                }

                char ch1_buf[64], ch2_buf[64];
                snprintf( ch1_buf, sizeof(ch1_buf), "Data: %u (0x%03X)", raw_sig, raw_sig );
                snprintf( ch2_buf, sizeof(ch2_buf), "Cnt: %u [%s]", counter, inv_ok ? "OK" : "INV ERR" );
                res.ch1_label = ch1_buf;
                res.ch2_label = ch2_buf;

                snprintf( buf, sizeof(buf), "Signal: %u | Rolling Counter: %u | Inverted Check: %s",
                          raw_sig, counter, inv_ok ? "PASS" : "FAIL" );
                res.tabular_summary = buf;
            }
            break;

        case PROFILE_SINGLE_16_A5:
            {
                /* 16-bit High Resolution Signal (D0..D3) + 8-bit Diagnostic / Counter (D4..D5) */
                U32 raw16 = ( (U32)d[0] << 12 ) | ( (U32)d[1] << 8 ) | ( (U32)d[2] << 4 ) | d[3];
                U8 diag = ( d[4] << 4 ) | d[5];

                char ch1_buf[64], ch2_buf[64];
                snprintf( ch1_buf, sizeof(ch1_buf), "Sig: %u (0x%04X)", raw16, raw16 );
                snprintf( ch2_buf, sizeof(ch2_buf), "Diag: 0x%02X", diag );
                res.ch1_label = ch1_buf;
                res.ch2_label = ch2_buf;

                snprintf( buf, sizeof(buf), "HighRes Signal: %u (0x%04X) | Diagnostic: 0x%02X", raw16, raw16, diag );
                res.tabular_summary = buf;
            }
            break;

        case PROFILE_RAW_12_12:
        default:
            {
                U16 fast1 = ( (U16)d[0] << 8 ) | ( (U16)d[1] << 4 ) | d[2];
                U16 fast2 = ( (U16)d[3] << 8 ) | ( (U16)d[4] << 4 ) | d[5];

                char ch1_buf[64], ch2_buf[64];
                snprintf( ch1_buf, sizeof(ch1_buf), "Fast1: %u (0x%03X)", fast1, fast1 );
                snprintf( ch2_buf, sizeof(ch2_buf), "Fast2: %u (0x%03X)", fast2, fast2 );
                res.ch1_label = ch1_buf;
                res.ch2_label = ch2_buf;

                snprintf( buf, sizeof(buf), "Fast1: %u (0x%03X) | Fast2: %u (0x%03X)", fast1, fast1, fast2, fast2 );
                res.tabular_summary = buf;
            }
            break;
    }

    return res;
}
