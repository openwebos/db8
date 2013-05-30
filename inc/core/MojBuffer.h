/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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


#ifndef MOJBUFFER_H_
#define MOJBUFFER_H_

#include "core/MojCoreDefs.h"
#include "core/MojAutoPtr.h"
#include "core/MojList.h"
#include "core/MojVector.h"

class MojBuffer : private MojNoCopy
{
public:
	class Chunk : public MojNoCopy
	{
	public:
		Chunk(gsize size);

		gsize dataSize() const { return m_dataEnd - m_begin; }
		gsize freeSpace() const { return m_end - m_dataEnd; }
		const guint8* data() const { return m_begin; }
		const guint8* dataEnd() const { return m_dataEnd; }
		guint8* data() { return m_begin; }
		guint8* dataEnd() { return m_dataEnd; }

		void write(const void* data, gsize size);
		void advance(gsize size) { MojAssert(size <= freeSpace()); m_dataEnd += size; }

		void* operator new(gsize objSize, gsize bufSize) { return MojMalloc(objSize + bufSize); }
		void operator delete(void *obj) { MojFree(obj); }

	private:
		friend class MojBuffer;

		guint8* m_begin;
		guint8* m_end;
		guint8* m_dataEnd;
		MojListEntry m_entry;
	};

	typedef MojVector<guint8> ByteVec;

	MojBuffer();
	MojBuffer(MojBuffer& buf);
	~MojBuffer();

	bool empty() const;
	void iovec(MojIoVecT* vec, gsize vecSize, gsize& vecSizeOut) const;
	MojErr toByteVec(ByteVec& vecOut) const;

	void clear();
	MojErr writeByte(guint8 b);
	MojErr write(const void* data, gsize size);
	MojErr data(const guint8*& dataOut, gsize& sizeOut);
	MojErr release(MojAutoPtr<Chunk>& chunkOut);
	void advance(gsize size);

	MojBuffer& operator=(MojBuffer& rhs);

private:
	static const gsize ChunkSize = 4096;
	typedef MojList<Chunk, &Chunk::m_entry> ChunkList;

	void pop();
	Chunk* allocChunk(gsize size) const;
	MojErr writeableChunk(Chunk*& chunkOut, gsize requestedSize);
	MojErr consolidate();

	ChunkList m_chunks;
	guint8* m_readPos;
};

#endif /* MOJBUFFER_H_ */
