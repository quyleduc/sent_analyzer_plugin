#ifndef SENT_ANALYZER_RESULTS
#define SENT_ANALYZER_RESULTS

#include <AnalyzerResults.h>

class SENTAnalyzer;
class SENTAnalyzerSettings;

enum SENTNibbleType {
    SyncPulse = 0,
    StatusNibble = 1,
    DataNibble = 2,
    CRCNibble = 3,
    PausePulse = 4,
    ErrorPulse = 5
};

enum SENTErrorType {
    NibbleNumberError = 0,
    CrcError = 1
};

class SENTAnalyzerResults : public AnalyzerResults
{
public:
    SENTAnalyzerResults( SENTAnalyzer* analyzer, SENTAnalyzerSettings* settings );
    virtual ~SENTAnalyzerResults();

    virtual void GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base );
    virtual void GenerateFrameTabularText( U64 frame_index, DisplayBase display_base );
    virtual void GeneratePacketTabularText( U64 packet_id, DisplayBase display_base );
    virtual void GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base );

    virtual void GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id );

protected:
    SENTAnalyzerSettings* mSettings;
    SENTAnalyzer* mAnalyzer;
};

#endif // SENT_ANALYZER_RESULTS
