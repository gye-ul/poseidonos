#include "src/array/partition/nvm_partition.h"

#include <gtest/gtest.h>

#include "src/include/array_config.h"
#include "src/include/pos_event_id.h"
#include "test/unit-tests/array/ft/buffer_entry_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/array/device/array_device_mock.h"

using ::testing::Return;
namespace pos
{
static LogicalBlkAddr
buildValidLogicalBlkAddr(uint32_t totalStripes, uint32_t blksPerStripe)
{
    LogicalBlkAddr lBlkAddr{
        .stripeId = totalStripes / 2,
        .offset = 0};
    return lBlkAddr;
}

static LogicalBlkAddr
buildInvalidLogicalBlkAddr(uint32_t totalStripes)
{
    LogicalBlkAddr lBlkAddr{
        .stripeId = totalStripes + 1,
        .offset = 0};
    return lBlkAddr;
}

static LogicalWriteEntry
buildValidLogicalWriteEntry(uint32_t totalStripes, uint32_t blksPerStripe)
{
    LogicalBlkAddr lBlkAddr{
        .stripeId = totalStripes / 2,
        .offset = 0};

    std::list<BufferEntry>* fakeBuffers = new std::list<BufferEntry>;
    MockBufferEntry mockBuffer(nullptr, 0, false);
    fakeBuffers->push_back(mockBuffer);

    LogicalWriteEntry lWriteEntry{
        .addr = lBlkAddr,
        .blkCnt = blksPerStripe / 2,
        .buffers = fakeBuffers};

    return lWriteEntry;
}

static LogicalByteWriteEntry
buildValidLogicalByteWriteEntry(uint32_t totalStripes, uint32_t byteCnt)
{
    LogicalBlkAddr lBlkAddr
    {
        .stripeId = totalStripes / 2,
        .offset = 0
    };

    LogicalByteAddr lByteAddr
    {
        .blkAddr = lBlkAddr,
        .byteOffset = 0,
        .byteSize = byteCnt
    };

    std::list<BufferEntry>* fakeBuffers = new std::list<BufferEntry>;
    MockBufferEntry mockBuffer(nullptr, 0, false);
    fakeBuffers->push_back(mockBuffer);

    LogicalByteWriteEntry lWriteEntry
    {
        .addr = lByteAddr,
        .byteCnt = byteCnt,
        .buffers = fakeBuffers
    };

    return lWriteEntry;
}

static LogicalWriteEntry
buildInvalidLogicalWriteEntry(uint32_t totalStripes, uint32_t blksPerStripe)
{
    LogicalBlkAddr lBlkAddr{
        .stripeId = totalStripes / 2,
        .offset = 0};

    LogicalWriteEntry lWriteEntry{
        .addr = lBlkAddr,
        .blkCnt = blksPerStripe + 1, // intentionally making it too big
        .buffers = nullptr};

    return lWriteEntry;
}

TEST(NvmPartition, NvmPartition_testIfConstructorInitializesLogicalSizeProperly)
{
    // Given
    PartitionPhysicalSize partPhySize{
        .startLba = 0/* not interesting */,
        .blksPerChunk = 100,
        .chunksPerStripe = 10,
        .stripesPerSegment = 5,
        .totalSegments = 2};
    vector<ArrayDevice*> devs;

    // When
    NvmPartition nvmPart("mock-array", 0, PartitionType::META_NVM, partPhySize, devs);

    // Then
    const PartitionLogicalSize* pLogicalSize = nvmPart.GetLogicalSize();
    ASSERT_EQ(1, pLogicalSize->minWriteBlkCnt);
    ASSERT_EQ(partPhySize.blksPerChunk, pLogicalSize->blksPerChunk);
    ASSERT_EQ(partPhySize.blksPerChunk * partPhySize.chunksPerStripe, pLogicalSize->blksPerStripe);
    ASSERT_EQ(partPhySize.stripesPerSegment * partPhySize.totalSegments, pLogicalSize->totalStripes);
    ASSERT_EQ(partPhySize.totalSegments, pLogicalSize->totalSegments);
    ASSERT_EQ(partPhySize.stripesPerSegment, pLogicalSize->stripesPerSegment);
}

TEST(NvmPartition, Translate_testIfInvalidAddressReturnsError)
{
    // Given
    PartitionPhysicalSize partPhySize{
        .startLba = 0/* not interesting */,
        .blksPerChunk = 100,
        .chunksPerStripe = 10,
        .stripesPerSegment = 5,
        .totalSegments = 2};
    uint32_t totalStripes = partPhySize.stripesPerSegment * partPhySize.totalSegments;
    uint32_t blksPerStripe = partPhySize.blksPerChunk * partPhySize.chunksPerStripe;
    LogicalBlkAddr invalidAddr = buildInvalidLogicalBlkAddr(totalStripes);
    vector<ArrayDevice*> devs;

    NvmPartition nvmPart("mock-array", 0, PartitionType::META_NVM, partPhySize, devs);
    PhysicalBlkAddr ignored;

    // When
    int actual = nvmPart.Translate(ignored, invalidAddr);

    // Then
    ASSERT_EQ(EID(ARRAY_INVALID_ADDRESS_ERROR), actual);
}

TEST(NvmPartition, Translate_testIfValidAddressIsFilledIn)
{
    // Given
    PartitionPhysicalSize partPhySize{
        .startLba = 8192,
        .blksPerChunk = 100,
        .chunksPerStripe = 10,
        .stripesPerSegment = 5,
        .totalSegments = 2};
    uint32_t totalStripes = partPhySize.stripesPerSegment * partPhySize.totalSegments;
    uint32_t blksPerStripe = partPhySize.blksPerChunk * partPhySize.chunksPerStripe;
    LogicalBlkAddr validAddr = buildValidLogicalBlkAddr(totalStripes, blksPerStripe);
    vector<ArrayDevice*> devs;
    devs.push_back(nullptr); // putting dummy 'cause I'm not interested

    NvmPartition nvmPart("mock-array", 0, PartitionType::META_NVM, partPhySize, devs);
    PhysicalBlkAddr dest;

    // When
    int actual = nvmPart.Translate(dest, validAddr);

    // Then
    ASSERT_EQ(0, actual);
    int expectedSrcBlock = totalStripes / 2 * nvmPart.GetLogicalSize()->blksPerStripe;
    int expectedSrcSector = expectedSrcBlock * ArrayConfig::SECTORS_PER_BLOCK;
    int expectedDestSector = expectedSrcSector + partPhySize.startLba;
    ASSERT_EQ(expectedDestSector, dest.lba);
}

TEST(NvmPartition, ByteTranslate_testIfInvalidAddressReturnsError)
{
    // Given
    PartitionPhysicalSize partPhySize{
        .startLba = 0/* not interseting */,
        .blksPerChunk = 100,
        .chunksPerStripe = 10,
        .stripesPerSegment = 5,
        .totalSegments = 2};
    uint32_t totalStripes = partPhySize.stripesPerSegment * partPhySize.totalSegments;
    uint32_t blksPerStripe = partPhySize.blksPerChunk * partPhySize.chunksPerStripe;
    uint32_t testByteOffset = 10;
    LogicalBlkAddr logicalByteAddr = buildInvalidLogicalBlkAddr(totalStripes);
    LogicalByteAddr invalidAddr;
    invalidAddr.blkAddr = logicalByteAddr;
    invalidAddr.byteOffset = testByteOffset;
    vector<ArrayDevice*> devs;

    NvmPartition nvmPart("mock-array", 0, PartitionType::META_NVM, partPhySize, devs);
    PhysicalByteAddr ignored;

    // When
    int actual = nvmPart.ByteTranslate(ignored, invalidAddr);

    // Then
    ASSERT_EQ(EID(ARRAY_INVALID_ADDRESS_ERROR), actual);
}

TEST(NvmPartition, ByteTranslate_testIfValidAddressIsFilledIn)
{
    // Given
    PartitionPhysicalSize partPhySize{
        .startLba = 8192,
        .blksPerChunk = 100,
        .chunksPerStripe = 10,
        .stripesPerSegment = 5,
        .totalSegments = 2};
    uint32_t totalStripes = partPhySize.stripesPerSegment * partPhySize.totalSegments;
    uint32_t blksPerStripe = partPhySize.blksPerChunk * partPhySize.chunksPerStripe;
    uint32_t testByteOffset = 5;
    LogicalBlkAddr logicalblkAddr = buildValidLogicalBlkAddr(totalStripes, blksPerStripe);
    LogicalByteAddr validAddr;
    validAddr.blkAddr = logicalblkAddr;
    validAddr.byteOffset = testByteOffset;
    validAddr.byteSize = 10;
    vector<ArrayDevice*> devs;
    string mockDevName = "mockDev";
    MockUBlockDevice* mockUblockDevice = new MockUBlockDevice(mockDevName, 1024, NULL);
    MockArrayDevice* mockArrayDevice = new MockArrayDevice(NULL);
    devs.push_back(mockArrayDevice);

    EXPECT_CALL(*mockArrayDevice, GetUblockPtr).WillRepeatedly(Return(mockUblockDevice));
    EXPECT_CALL(*mockUblockDevice, GetByteAddress).WillOnce(Return((void*)0));

    NvmPartition nvmPart("mock-array", 0, PartitionType::META_NVM, partPhySize, devs);
    PhysicalByteAddr dest;

    // When
    int actual = nvmPart.ByteTranslate(dest, validAddr);

    // Then
    ASSERT_EQ(0, actual);
    int expectedSrcBlock = totalStripes / 2 * nvmPart.GetLogicalSize()->blksPerStripe;
    int expectedSrcSector = expectedSrcBlock * ArrayConfig::SECTORS_PER_BLOCK;
    int expectedDestByte = (expectedSrcSector + partPhySize.startLba) * ArrayConfig::SECTOR_SIZE_BYTE + testByteOffset;
    ASSERT_EQ(expectedDestByte, dest.byteAddress);

    delete mockArrayDevice;
    delete mockUblockDevice;
}

TEST(NvmPartition, Convert_testIfInvalidEntryReturnsError)
{
    // Given
    PartitionPhysicalSize partPhySize{
        .startLba = 0/* not interesting */,
        .blksPerChunk = 100,
        .chunksPerStripe = 10,
        .stripesPerSegment = 5,
        .totalSegments = 2};
    uint32_t totalStripes = partPhySize.stripesPerSegment * partPhySize.totalSegments;
    uint32_t blksPerStripe = partPhySize.blksPerChunk * partPhySize.chunksPerStripe;
    LogicalWriteEntry invalidEntry = buildInvalidLogicalWriteEntry(totalStripes, blksPerStripe);
    vector<ArrayDevice*> devs;
    NvmPartition nvmPart("mock-array", 0, PartitionType::META_NVM, partPhySize, devs);
    std::list<PhysicalWriteEntry> ignored;

    // When
    int actual = nvmPart.Convert(ignored, invalidEntry);

    // Then
    ASSERT_EQ(EID(ARRAY_INVALID_ADDRESS_ERROR), actual);
}

TEST(NvmPartition, Convert_testIfValidEntryIsFilledIn)
{
    // Given
    PartitionPhysicalSize partPhySize{
        .startLba = 8192,
        .blksPerChunk = 100,
        .chunksPerStripe = 10,
        .stripesPerSegment = 5,
        .totalSegments = 2};
    uint32_t totalStripes = partPhySize.stripesPerSegment * partPhySize.totalSegments;
    uint32_t blksPerStripe = partPhySize.blksPerChunk * partPhySize.chunksPerStripe;
    LogicalWriteEntry validEntry = buildValidLogicalWriteEntry(totalStripes, blksPerStripe);
    vector<ArrayDevice*> devs;
    devs.push_back(nullptr); // 'cause I'm not interested
    NvmPartition nvmPart("mock-array", 0, PartitionType::META_NVM, partPhySize, devs);
    std::list<PhysicalWriteEntry> dest;

    // When
    int actual = nvmPart.Convert(dest, validEntry);

    // Then
    ASSERT_EQ(1, dest.size());
    PhysicalWriteEntry pWriteEntry = dest.front();
    ASSERT_EQ(validEntry.blkCnt, pWriteEntry.blkCnt);
    PhysicalBlkAddr phyBlkAddr = pWriteEntry.addr;
    // The validation on phyBlkAddr will be skipped for now since it's already done as part of Translate() UT.
}

TEST(NvmPartition, ByteConvert_testIfValidEntryIsFilledIn)
{
    // Given
    PartitionPhysicalSize partPhySize{
        .startLba = 8192,
        .blksPerChunk = 100,
        .chunksPerStripe = 10,
        .stripesPerSegment = 5,
        .totalSegments = 2};
    uint32_t totalStripes = partPhySize.stripesPerSegment * partPhySize.totalSegments;
    uint32_t blkCnt = 10; // bytes to convert
    LogicalByteWriteEntry validEntry = buildValidLogicalByteWriteEntry(totalStripes, blkCnt);
    vector<ArrayDevice*> devs;
    string mockDevName = "mockDev";
    MockUBlockDevice* mockUblockDevice = new MockUBlockDevice(mockDevName, 1024, NULL);
    MockArrayDevice* mockArrayDevice = new MockArrayDevice(NULL);
    devs.push_back(mockArrayDevice);

    EXPECT_CALL(*mockArrayDevice, GetUblockPtr).WillRepeatedly(Return(mockUblockDevice));
    EXPECT_CALL(*mockUblockDevice, GetByteAddress).WillOnce(Return((void*)0));

    NvmPartition nvmPart("mock-array", 0, PartitionType::META_NVM, partPhySize, devs);
    std::list<PhysicalByteWriteEntry> dest;

    // When
    int actual = nvmPart.ByteConvert(dest, validEntry);

    // Then
    ASSERT_EQ(1, dest.size());
    PhysicalByteWriteEntry pWriteEntry = dest.front();
    ASSERT_EQ(validEntry.byteCnt, pWriteEntry.byteCnt);
    PhysicalByteAddr phyByteAddr = pWriteEntry.addr;

    delete mockArrayDevice;
    delete mockUblockDevice;
}

TEST(NvmPartition, IsByteAccessSupported_testIfReturnValueCorrect)
{
    // Given
    PartitionPhysicalSize partPhySize{
        .startLba = 8192,
        .blksPerChunk = 100,
        .chunksPerStripe = 10,
        .stripesPerSegment = 5,
        .totalSegments = 2};
    vector<ArrayDevice*> devs;
    devs.push_back(nullptr); // 'cause I'm not interested
    NvmPartition nvmPart("mock-array", 0, PartitionType::META_NVM, partPhySize, devs);

    // When
    bool actual = nvmPart.IsByteAccessSupported();

    // Then
    ASSERT_TRUE(actual);
}

TEST(NvmPartition, GetMethod_testIfNoMethodReturnedForNvmPartition)
{
    // Given
    PartitionPhysicalSize partPhySize{
        .startLba = 8192,
        .blksPerChunk = 100,
        .chunksPerStripe = 10,
        .stripesPerSegment = 5,
        .totalSegments = 2};
    vector<ArrayDevice*> devs;
    devs.push_back(nullptr); // 'cause I'm not interested
    NvmPartition nvmPart("mock-array", 0, PartitionType::META_NVM, partPhySize, devs);
    // When
    Method* actual = nvmPart.GetMethod();
    // Then
    ASSERT_EQ(nullptr, actual);
}

TEST(NvmPartition, Format_dummyTestForCoverage)
{
    // Given
    PartitionPhysicalSize partPhySize{
        .startLba = 8192,
        .blksPerChunk = 100,
        .chunksPerStripe = 10,
        .stripesPerSegment = 5,
        .totalSegments = 2};
    vector<ArrayDevice*> devs;
    devs.push_back(nullptr); // 'cause I'm not interested
    NvmPartition nvmPart("mock-array", 0, PartitionType::META_NVM, partPhySize, devs);
    // When
    nvmPart.Format(); // nothing happens
    // Then
}

TEST(NvmPartition, Include_testCopyOperatorOfIncludedStructure)
{
    // Given
    PhysicalWriteEntry pwe1;
    PhysicalWriteEntry pwe2;
    // When
    pwe1 = pwe2;
}

} // namespace pos
