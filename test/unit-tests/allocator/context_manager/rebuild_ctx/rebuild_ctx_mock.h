#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"

namespace pos
{
class MockRebuildCtx : public RebuildCtx
{
public:
    using RebuildCtx::RebuildCtx;
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, AfterLoad, (char* buf), (override));
    MOCK_METHOD(void, BeforeFlush, (char* buf), (override));
    MOCK_METHOD(std::mutex&, GetCtxLock, (), (override));
    MOCK_METHOD(void, FinalizeIo, (AsyncMetaFileIoCtx* ctx), (override));
    MOCK_METHOD(char*, GetSectionAddr, (int section), (override));
    MOCK_METHOD(int, GetSectionSize, (int section), (override));
    MOCK_METHOD(uint64_t, GetStoredVersion, (), (override));
    MOCK_METHOD(void, ResetDirtyVersion, (), (override));
    MOCK_METHOD(std::string, GetFilename, (), (override));
    MOCK_METHOD(uint32_t, GetSignature, (), (override));
    MOCK_METHOD(int, GetNumSections, (), (override));
    MOCK_METHOD(SegmentId, GetRebuildTargetSegment, (), (override));
    MOCK_METHOD(int, ReleaseRebuildSegment, (SegmentId segmentId), (override));
    MOCK_METHOD(bool, NeedRebuildAgain, (), (override));
    MOCK_METHOD(int, FreeSegmentInRebuildTarget, (SegmentId segId), (override));
    MOCK_METHOD(bool, IsRebuidTargetSegmentsEmpty, (), (override));
    MOCK_METHOD(bool, IsRebuildTargetSegment, (SegmentId segId), (override));
    MOCK_METHOD(uint32_t, GetRebuildTargetSegmentCount, (), (override));
    MOCK_METHOD(RTSegmentIter, GetRebuildTargetSegmentsBegin, (), (override));
    MOCK_METHOD(RTSegmentIter, GetRebuildTargetSegmentsEnd, (), (override));
    MOCK_METHOD(void, ClearRebuildTargetList, (), (override));
    MOCK_METHOD(void, AddRebuildTargetSegment, (SegmentId segmentId), (override));
    MOCK_METHOD(int, StopRebuilding, (), (override));
    MOCK_METHOD(void, EraseRebuildTargetSegment, (SegmentId segmentId), (override));
    MOCK_METHOD(std::mutex&, GetLock, (), (override));
    MOCK_METHOD((std::pair<RTSegmentIter, bool>), EmplaceRebuildTargetSegment, (SegmentId segmentId), (override));
    MOCK_METHOD(void, SetTargetSegmentCnt, (uint32_t val), (override));
};

} // namespace pos
