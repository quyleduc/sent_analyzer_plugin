#ifndef SENT_PROFILES_H
#define SENT_PROFILES_H

#include <LogicPublicTypes.h>
#include <string>
#include <vector>

enum SENTProfileType {
    PROFILE_RAW_12_12 = 0,         /* Raw 12-bit Fast1 + 12-bit Fast2 */
    PROFILE_DUAL_THROTTLE_A1 = 1,  /* SAE J2716 A.1: Dual Throttle Position Sensor (TPS1 + TPS2 %) */
    PROFILE_TMAP_A2 = 2,           /* SAE J2716 A.2: Pressure (kPa) & Temperature (°C) */
    PROFILE_MAF_A3 = 3,            /* SAE J2716 A.3: Mass Air Flow (14-bit kg/h) & Temperature (10-bit °C) */
    PROFILE_SECURE_A4 = 4,         /* SAE J2716 A.4: 12-bit Data + 8-bit Rolling Counter + 4-bit Inverted */
    PROFILE_SINGLE_16_A5 = 5       /* SAE J2716 A.5: Single 16-bit High-Resolution Signal + 8-bit Diag */
};

struct DecodedProfileResult {
    std::string profile_name;
    std::string tabular_summary;   /* Short summary for Data Table */
    std::string ch1_label;         /* e.g. "TPS1: 25.0%" */
    std::string ch2_label;         /* e.g. "TPS2: 75.0%" */
    bool has_safety_alert;         /* e.g. Redundancy mismatch or counter discontinuity */
    std::string alert_msg;
};

class SENTProfiles {
public:
    static const char* GetProfileName( SENTProfileType type );
    static DecodedProfileResult DecodeFrame( SENTProfileType type, const std::vector<U8>& data_nibbles, U8 status_val );
};

#endif // SENT_PROFILES_H
