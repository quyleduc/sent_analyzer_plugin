#ifndef SENT_ANALYZER_H
#define SENT_ANALYZER_H

#include <Analyzer.h>
#include "SENTAnalyzerResults.h"
#include "SENTSimulationDataGenerator.h"
#include <vector>

class SENTAnalyzerSettings;
class SENTAnalyzer : public Analyzer2
{
public:
    SENTAnalyzer();
    virtual ~SENTAnalyzer();

    virtual void SetupResults();
    virtual void WorkerThread();

    virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
    virtual U32 GetMinimumSampleRateHz();

    virtual const char* GetAnalyzerName() const;
    virtual bool NeedsRerun();

protected:
    U8 CalculateCRC( U8 status_val, const std::vector<U8>& data_nibbles );

    std::unique_ptr<SENTAnalyzerSettings> mSettings;
    std::unique_ptr<SENTAnalyzerResults> mResults;
    AnalyzerChannelData* mSerial;

    SENTSimulationDataGenerator mSimulationDataGenerator;
    bool mSimulationInitialized;

    double mSamplesPerTick;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif // SENT_ANALYZER_H
