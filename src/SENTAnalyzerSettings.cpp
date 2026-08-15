#include "SENTAnalyzerSettings.h"
#include <AnalyzerHelpers.h>

SENTAnalyzerSettings::SENTAnalyzerSettings()
:   mInputChannel( UNDEFINED_CHANNEL ),
    mTickTimeHalfUs( 6 ),                 /* Default: 6 half-us = 3.0 us */
    mPausePulseEnabled( true ),           /* Default: Pause Pulse enabled */
    mNumberOfDataNibbles( 6 ),            /* Default: 6 data nibbles */
    mLegacyCrc( true ),                   /* Default: Legacy CRC */
    mProfileType( PROFILE_RAW_12_12 )     /* Default: Raw 12+12 */
{
    mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
    mInputChannelInterface->SetTitleAndTooltip( "Serial", "Standard SENT (SAE J2716)" );
    mInputChannelInterface->SetChannel( mInputChannel );

    mTickTimeInterface.reset( new AnalyzerSettingInterfaceInteger() );
    mTickTimeInterface->SetTitleAndTooltip( "tick time (half us)", "Specify the SENT tick time in half microseconds (e.g. 6 = 3.0 us)" );
    mTickTimeInterface->SetMax( 100 );
    mTickTimeInterface->SetMin( 1 );
    mTickTimeInterface->SetInteger( mTickTimeHalfUs );

    mPausePulseInterface.reset( new AnalyzerSettingInterfaceBool() );
    mPausePulseInterface->SetTitleAndTooltip( "Pause pulse", "Specify whether pause pulse is enabled" );
    mPausePulseInterface->SetValue( mPausePulseEnabled );

    mNumberOfDataNibblesInterface.reset( new AnalyzerSettingInterfaceInteger() );
    mNumberOfDataNibblesInterface->SetTitleAndTooltip( "Number of data nibbles", "Specify number of fast channel data nibbles (e.g. 6)" );
    mNumberOfDataNibblesInterface->SetMax( 6 );
    mNumberOfDataNibblesInterface->SetMin( 1 );
    mNumberOfDataNibblesInterface->SetInteger( mNumberOfDataNibbles );

    mLegacyCrcInterface.reset( new AnalyzerSettingInterfaceBool() );
    mLegacyCrcInterface->SetTitleAndTooltip( "Legacy CRC", "Specify whether legacy CRC-4 (6 data nibbles, seed=5) is used" );
    mLegacyCrcInterface->SetValue( mLegacyCrc );

    mProfileInterface.reset( new AnalyzerSettingInterfaceNumberList() );
    mProfileInterface->SetTitleAndTooltip( "Sensor Profile", "Select SAE J2716 sensor profile for physical engineering unit decoding" );
    mProfileInterface->AddNumber( PROFILE_RAW_12_12, "Raw Fast Channels (12b + 12b)", "Default raw hex and decimal values" );
    mProfileInterface->AddNumber( PROFILE_DUAL_THROTTLE_A1, "A.1: Dual Throttle (TPS %)", "Dual throttle position with redundancy check" );
    mProfileInterface->AddNumber( PROFILE_TMAP_A2, "A.2: TMAP (Pressure & Temp)", "Pressure in kPa (20..300) and Temperature in °C (-40..+150)" );
    mProfileInterface->AddNumber( PROFILE_MAF_A3, "A.3: MAF (Flow 14b & Temp 10b)", "Mass Air Flow in kg/h (0..640) and Temp in °C (-40..+120)" );
    mProfileInterface->AddNumber( PROFILE_SECURE_A4, "A.4: Secure (12b + 8b Counter)", "12-bit Data + 8-bit Rolling Counter + 4-bit Inverted Check" );
    mProfileInterface->AddNumber( PROFILE_SINGLE_16_A5, "A.5: Single 16-bit High Res", "16-bit High Resolution Signal + 8-bit Diagnostic" );
    mProfileInterface->SetNumber( (double)mProfileType );

    AddInterface( mInputChannelInterface.get() );
    AddInterface( mTickTimeInterface.get() );
    AddInterface( mPausePulseInterface.get() );
    AddInterface( mNumberOfDataNibblesInterface.get() );
    AddInterface( mLegacyCrcInterface.get() );
    AddInterface( mProfileInterface.get() );

    AddExportOption( 0, "Export as text/csv file" );
    AddExportExtension( 0, "text", "txt" );
    AddExportExtension( 0, "csv", "csv" );

    ClearChannels();
    AddChannel( mInputChannel, "Serial", false );
}

SENTAnalyzerSettings::~SENTAnalyzerSettings()
{
}

bool SENTAnalyzerSettings::SetSettingsFromInterfaces()
{
    mInputChannel = mInputChannelInterface->GetChannel();
    mTickTimeHalfUs = mTickTimeInterface->GetInteger();
    mPausePulseEnabled = mPausePulseInterface->GetValue();
    mNumberOfDataNibbles = mNumberOfDataNibblesInterface->GetInteger();
    mLegacyCrc = mLegacyCrcInterface->GetValue();
    mProfileType = (SENTProfileType)(int)mProfileInterface->GetNumber();

    ClearChannels();
    AddChannel( mInputChannel, "SENT", true );

    return true;
}

void SENTAnalyzerSettings::UpdateInterfacesFromSettings()
{
    mInputChannelInterface->SetChannel( mInputChannel );
    mTickTimeInterface->SetInteger( mTickTimeHalfUs );
    mPausePulseInterface->SetValue( mPausePulseEnabled );
    mNumberOfDataNibblesInterface->SetInteger( mNumberOfDataNibbles );
    mLegacyCrcInterface->SetValue( mLegacyCrc );
    mProfileInterface->SetNumber( (double)mProfileType );
}

void SENTAnalyzerSettings::LoadSettings( const char* settings )
{
    SimpleArchive text_archive;
    text_archive.SetString( settings );

    U32 profile_num = 0;
    text_archive >> mInputChannel;
    text_archive >> mTickTimeHalfUs;
    text_archive >> mPausePulseEnabled;
    text_archive >> mNumberOfDataNibbles;
    text_archive >> mLegacyCrc;
    text_archive >> profile_num;
    mProfileType = (SENTProfileType)profile_num;

    ClearChannels();
    AddChannel( mInputChannel, "SENT", true );

    UpdateInterfacesFromSettings();
}

const char* SENTAnalyzerSettings::SaveSettings()
{
    SimpleArchive text_archive;

    text_archive << mInputChannel;
    text_archive << mTickTimeHalfUs;
    text_archive << mPausePulseEnabled;
    text_archive << mNumberOfDataNibbles;
    text_archive << mLegacyCrc;
    text_archive << (U32)mProfileType;

    return SetReturnString( text_archive.GetString() );
}
