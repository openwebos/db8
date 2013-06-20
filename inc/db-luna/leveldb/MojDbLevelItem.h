/* @@@LICENSE
*
* Copyright (c) 2013 LG Electronics, Inc.
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

#ifndef MOJDBLEVELITEM_H
#define MOJDBLEVELITEM_H

#include <leveldb/db.h>
#include "db/MojDbStorageEngine.h"
#include "db/MojDbObjectHeader.h"

class MojDbLevelItem : public MojDbStorageItem
{
public:
    MojDbLevelItem();
    virtual ~MojDbLevelItem() { freeData(); }
    virtual MojErr close() { return MojErrNone; }
    virtual MojErr kindId(MojString& kindIdOut, MojDbKindEngine& kindEngine);
    virtual MojErr visit(MojObjectVisitor& visitor, MojDbKindEngine& kindEngine, bool headerExpected = true) const;
    virtual const MojObject& id() const { return m_header.id(); }
    virtual MojSize size() const { return m_slice.size(); }

    void clear();
    const MojByte* data() const { return (const MojByte*) m_slice.data(); }
    bool hasPrefix(const MojDbKey& prefix) const;
    MojErr toArray(MojObject& arrayOut) const;
    MojErr toObject(MojObject& objOut) const;

    void id(const MojObject& id);
    void fromBytesNoCopy(const MojByte* bytes, MojSize size);
    MojErr fromBuffer(MojBuffer& buf);
    MojErr fromBytes(const MojByte* bytes, MojSize size);
    MojErr fromObject(const MojObject& obj);
    MojErr fromObjectVector(const MojVector<MojObject>& vec);
    MojErr from(const leveldb::Slice &slice) { return fromBytes(reinterpret_cast<const MojByte*>(slice.data()), slice.size()); }

    const leveldb::Slice* impl() { return &m_slice; }

private:
    void freeData();
    void setData(MojByte* bytes, MojSize size, void (*free)(void*));

    // either points to m_chunk or to m_data
    leveldb::Slice m_slice;
    MojAutoPtr<MojBuffer::Chunk> m_chunk;
    MojByte *m_data;
    mutable MojDbObjectHeader m_header;
    void (*m_free)(void*);
};

#endif

