#ifndef SENT_ANALYZER_SETTINGS
#define SENT_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class SENTAnalyzerSettings : public AnalyzerSettings
{
public:
    SENTAnalyzerSettings();
    virtual ~SENTAnalyzerSettings();

    virtual bool SetSettingsFromInterfaces();
    void UpdateInterfacesFromSettings();
    virtual void LoadSettings( const char* settings );
    virtual const char* SaveSettings();

    Channel mInputChannel;
    U32 mTickTimeHalfUs;       /* 1..100 half-microseconds (e.g. 6 = 3.0 us) */
    bool mPausePulseEnabled;    /* true: 10 pulses/frame with pause pulse */
    U32 mNumberOfDataNibbles;  /* 1..6 data nibbles */
    bool mLegacyCrc;           /* true: Legacy CRC (6 nibbles, seed=5); false: APR2016 (7 nibbles, seed=3) */

protected:
    std::unique_ptr<AnalyzerSettingInterfaceChannel> mInputChannelInterface;
    std::unique_ptr<AnalyzerSettingInterfaceInteger> mTickTimeInterface;
    std::unique_ptr<AnalyzerSettingInterfaceBool>    mPausePulseInterface;
    std::unique_ptr<AnalyzerSettingInterfaceInteger> mNumberOfDataNibblesInterface;
    std::unique_ptr<AnalyzerSettingInterfaceBool>    mLegacyCrcInterface;
};

#endif // SENT_ANALYZER_SETTINGS
