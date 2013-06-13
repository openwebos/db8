/* @@@LICENSE
*
*  Copyright (c) 2009-2013 LG Electronics, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */

#include "db-luna/leveldb/MojDbLevelItem.h"
#include "db/MojDbKindEngine.h"
#include "core/MojObjectBuilder.h"

////////////////////MojDbLevelItem////////////////////////////////////////////

MojDbLevelItem::MojDbLevelItem()
: m_data(NULL), m_free(MojFree)
{
}

MojErr MojDbLevelItem::kindId(MojString& kindIdOut, MojDbKindEngine& kindEngine)
{
    MojErr err = m_header.read(kindEngine);
    MojErrCheck(err);

    kindIdOut = m_header.kindId();

    return MojErrNone;
}

MojErr MojDbLevelItem::visit(MojObjectVisitor& visitor, MojDbKindEngine& kindEngine, bool headerExpected) const
{
    MojErr err = MojErrNone;
    MojTokenSet tokenSet;
    if (headerExpected) {
        err = m_header.visit(visitor, kindEngine);
        MojErrCheck(err);
        const MojString& kindId = m_header.kindId();
        err = kindEngine.tokenSet(kindId, tokenSet);
        MojErrCheck(err);
        m_header.reader().tokenSet(&tokenSet);
        err = m_header.reader().read(visitor);
        MojErrCheck(err);
    } else {
        MojObjectReader reader(data(), size());
        err = reader.read(visitor);
        MojErrCheck(err);
    }
    return MojErrNone;
}

void MojDbLevelItem::id(const MojObject& id)
{
    m_header.reset();
    m_header.id(id);
    m_header.reader().data(data(), size());
}

void MojDbLevelItem::clear()
{
    freeData();
    m_slice.clear();
}

bool MojDbLevelItem::hasPrefix(const MojDbKey& prefix) const
{
    return (size() >= prefix.size() &&
            MojMemCmp(data(), prefix.data(), prefix.size()) == 0);
}

MojErr MojDbLevelItem::toArray(MojObject& arrayOut) const
{
    MojObjectBuilder builder;
    MojErr err = builder.beginArray();
    MojErrCheck(err);
    err = MojObjectReader::read(builder, data(), size());
    MojErrCheck(err);
    err = builder.endArray();
    MojErrCheck(err);
    arrayOut = builder.object();

    return MojErrNone;
}

MojErr MojDbLevelItem::toObject(MojObject& objOut) const
{
    MojObjectBuilder builder;
    MojErr err = MojObjectReader::read(builder, data(), size());
    MojErrCheck(err);
    objOut = builder.object();

    return MojErrNone;
}

void MojDbLevelItem::fromBytesNoCopy(const MojByte* bytes, MojSize size)
{
    MojAssert(bytes || size == 0);
    MojAssert(size <= MojUInt32Max);

    setData(const_cast<MojByte*> (bytes), size, NULL);
}

MojErr MojDbLevelItem::fromBuffer(MojBuffer& buf)
{
    clear();
    if (!buf.empty()) {
        MojAutoPtr<MojBuffer::Chunk> chunk;
        MojErr err = buf.release(chunk);
        MojErrCheck(err);
        MojAssert(chunk.get());
        setData(chunk->data(), chunk->dataSize(), NULL);
        m_chunk = chunk; // take ownership
    }
    return MojErrNone;
}

MojErr MojDbLevelItem::fromBytes(const MojByte* bytes, MojSize size)
{
    MojAssert (bytes || size == 0);

    if (size == 0) {
        clear();
    } else {
        MojByte* newBytes = (MojByte*)MojMalloc(size);
        MojAllocCheck(newBytes);
        MojMemCpy(newBytes, bytes, size);
        setData(newBytes, size, MojFree);
    }
    return MojErrNone;
}

MojErr MojDbLevelItem::fromObject(const MojObject& obj)
{
    MojObjectWriter writer;
    MojErr err = obj.visit(writer);
    MojErrCheck(err);
    err = fromBuffer(writer.buf());
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLevelItem::fromObjectVector(const MojVector<MojObject>& vec)
{
    MojAssert(!vec.empty());

    MojObjectWriter writer;
    for (MojVector<MojObject>::ConstIterator i = vec.begin();
        i != vec.end();
        ++i) {
        MojErr err = i->visit(writer);
        MojErrCheck(err);
    }
    fromBuffer(writer.buf());

    return MojErrNone;
}

void MojDbLevelItem::freeData()
{
    // make sure slice points to something weird rather than to unallocated
    // memory
    m_slice = leveldb::Slice("(uninitialized)");

    // reset header reader
    m_header.reset();

    // free m_data
    if (m_free)
    {
        m_free(m_data);
        m_data = NULL;
    }
    m_free = MojFree;

    // free m_chunk
    m_chunk.reset();
}

void MojDbLevelItem::setData(MojByte* bytes, MojSize size, void (*free)(void*))
{
    MojAssert(bytes);
    freeData();
    m_data = bytes;
    m_free = free;
    m_slice = leveldb::Slice( (const char *)bytes, size);
    m_header.reader().data(bytes, size);
}
