#include "SENTAnalyzerSettings.h"
#include <AnalyzerHelpers.h>

SENTAnalyzerSettings::SENTAnalyzerSettings()
:   mInputChannel( UNDEFINED_CHANNEL ),
    mTickTimeHalfUs( 6 ),         /* Default: 6 half-us = 3.0 us */
    mPausePulseEnabled( true ),   /* Default: Pause Pulse enabled */
    mNumberOfDataNibbles( 6 ),    /* Default: 6 data nibbles */
    mLegacyCrc( true )            /* Default: Legacy CRC */
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

    AddInterface( mInputChannelInterface.get() );
    AddInterface( mTickTimeInterface.get() );
    AddInterface( mPausePulseInterface.get() );
    AddInterface( mNumberOfDataNibblesInterface.get() );
    AddInterface( mLegacyCrcInterface.get() );

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
}

void SENTAnalyzerSettings::LoadSettings( const char* settings )
{
    SimpleArchive text_archive;
    text_archive.SetString( settings );

    text_archive >> mInputChannel;
    text_archive >> mTickTimeHalfUs;
    text_archive >> mPausePulseEnabled;
    text_archive >> mNumberOfDataNibbles;
    text_archive >> mLegacyCrc;

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

    return SetReturnString( text_archive.GetString() );
}
