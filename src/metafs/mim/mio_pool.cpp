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

#include "mio_pool.h"

#include <vector>

#include "metafs_common.h"

namespace pos
{
MioPool::MioPool(MpioPool* mpioPool, uint32_t poolSize)
{
    assert(mpioPool != nullptr && poolSize != 0);
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "MioPool poolsize={}", poolSize);

    while (poolSize-- != 0)
    {
        Mio* mio = new Mio(mpioPool);
        mio->InitStateHandler();
        mioList.push_back(mio);
    }
}

MioPool::~MioPool(void)
{
    _FreeAllMioinPool();
    mioList.clear();
    mioTagIdAllocator.Reset();
}

Mio*
MioPool::Alloc(void)
{
    if (0 == mioList.size())
        return nullptr;

    Mio* mio = mioList.back();
    mioList.pop_back();

    return mio;
}

bool
MioPool::IsEmpty(void)
{
    return mioList.empty();
}

void
MioPool::Release(Mio* mio)
{
    mio->Reset();
    mioList.push_back(mio);
}

void
MioPool::_FreeAllMioinPool(void)
{
    for (std::vector<Mio*>::iterator itr = mioList.begin(); itr != mioList.end(); ++itr)
    {
        delete *itr;
        *itr = nullptr;
    }
}
} // namespace pos
