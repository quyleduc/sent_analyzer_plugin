#include "SENTSimulationDataGenerator.h"
#include "SENTAnalyzerSettings.h"
#include <AnalyzerHelpers.h>

SENTSimulationDataGenerator::SENTSimulationDataGenerator()
:   mSettings( nullptr ),
    mSimulationSampleRateHz( 0 ),
    mStringIndex( 0 )
{
}

SENTSimulationDataGenerator::~SENTSimulationDataGenerator()
{
}

void SENTSimulationDataGenerator::Initialize( U32 simulation_sample_rate, SENTAnalyzerSettings* settings )
{
    mSimulationSampleRateHz = simulation_sample_rate;
    mSettings = settings;

    mSerialSimulationData.SetChannel( mSettings->mInputChannel );
    mSerialSimulationData.SetSampleRate( simulation_sample_rate );
    mSerialSimulationData.SetInitialBitState( BIT_HIGH );
}

U32 SENTSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel )
{
    U64 adjusted_largest_sample_requested = largest_sample_requested;

    while( mSerialSimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested )
    {
        CreateSerialByte();
    }

    *simulation_channel = &mSerialSimulationData;
    return 1;
}

void SENTSimulationDataGenerator::CreateSerialByte()
{
    double tick_samples = mSimulationSampleRateHz * 3.0e-6; /* 3.0 us / tick */
    U32 low_samples = (U32)(5.0 * tick_samples);

    /* 1. Sync pulse (56 ticks) */
    mSerialSimulationData.Transition(); /* High -> Low */
    mSerialSimulationData.Advance( low_samples );
    mSerialSimulationData.Transition(); /* Low -> High */
    mSerialSimulationData.Advance( (U32)(51.0 * tick_samples) );

    /* 2. Status nibble (val=0 -> 12 ticks) */
    mSerialSimulationData.Transition();
    mSerialSimulationData.Advance( low_samples );
    mSerialSimulationData.Transition();
    mSerialSimulationData.Advance( (U32)(7.0 * tick_samples) );

    /* 3. Fast Data 1 = 0x400 (4, 0, 0) & Fast Data 2 = 0x800 (8, 0, 0) */
    U8 nibbles[6] = {4, 0, 0, 8, 0, 0};
    for( int i = 0; i < 6; i++ )
    {
        mSerialSimulationData.Transition();
        mSerialSimulationData.Advance( low_samples );
        mSerialSimulationData.Transition();
        mSerialSimulationData.Advance( (U32)((7.0 + nibbles[i]) * tick_samples) );
    }

    /* 4. CRC = 0xA (10 -> 22 ticks) */
    mSerialSimulationData.Transition();
    mSerialSimulationData.Advance( low_samples );
    mSerialSimulationData.Transition();
    mSerialSimulationData.Advance( (U32)(17.0 * tick_samples) );

    /* 5. Pause pulse = 110 ticks */
    mSerialSimulationData.Transition();
    mSerialSimulationData.Advance( low_samples );
    mSerialSimulationData.Transition();
    mSerialSimulationData.Advance( (U32)(105.0 * tick_samples) );
}
