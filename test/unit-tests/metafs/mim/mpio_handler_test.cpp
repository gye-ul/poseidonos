/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/metafs/mim/mpio_handler.h"
#include "test/unit-tests/metafs/mim/metafs_io_q_mock.h"
#include "test/unit-tests/metafs/mim/mpio_pool_mock.h"
#include "test/unit-tests/metafs/mim/write_mpio_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::NiceMock;

namespace pos
{
TEST(MpioHandler, Normal)
{
    const int MAX_COUNT = 32 * 1024;

    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>;

    MockMpioPool* pool = new MockMpioPool(100);
    EXPECT_CALL(*pool, GetPoolSize);
#if MPIO_CACHE_EN
    EXPECT_CALL(*pool, ReleaseCache).WillRepeatedly(Return());
#endif
    MockWriteMpio* mpio = new MockWriteMpio(this);
    EXPECT_CALL(*mpio, ExecuteAsyncState).WillRepeatedly(Return());

    MockMetaFsIoQ<Mpio*>* doneQ = new MockMetaFsIoQ<Mpio*>();
    EXPECT_CALL(*doneQ, Init);
    EXPECT_CALL(*doneQ, Enqueue).WillRepeatedly(Return(true));
    EXPECT_CALL(*doneQ, Dequeue).WillRepeatedly(Return(mpio));
    EXPECT_CALL(*doneQ, GetItemCnt).WillRepeatedly(Return(0));

    MpioHandler* handler = new MpioHandler(0, 0, tp, doneQ);
    handler->BindMpioPool(pool);

    for (int i = 0; i < MAX_COUNT; i++)
    {
        handler->EnqueuePartialMpio(mpio);
    }

    for (int i = 0; i < MAX_COUNT; i++)
    {
        handler->BottomhalfMioProcessing();
    }

    delete handler;
    delete pool;
    delete mpio;
    delete tp;
}

} // namespace pos
