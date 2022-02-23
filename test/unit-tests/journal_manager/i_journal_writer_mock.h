#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/i_journal_writer.h"

namespace pos
{
class MockIJournalWriter : public IJournalWriter
{
public:
    using IJournalWriter::IJournalWriter;
    MOCK_METHOD(int, AddBlockMapUpdatedLog, (VolumeIoSmartPtr volumeIo, MpageList dirty, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(int, AddStripeMapUpdatedLog, (Stripe* stripe, StripeAddr oldAddr, MpageList dirty, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(int, AddGcStripeFlushedLog, (GcStripeMapUpdateList mapUpdates, MapPageList dirty, EventSmartPtr callbackEvent), (override));
};

} // namespace pos
