/* @@@LICENSE
*
*      Copyright (c) 2009-2013 LG Electronics, Inc.
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

#include "core/MojLog.h"
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
		Chunk(MojSize size);

		MojSize dataSize() const { return m_dataEnd - m_begin; }
		MojSize freeSpace() const { return m_end - m_dataEnd; }
		const MojByte* data() const { return m_begin; }
		const MojByte* dataEnd() const { return m_dataEnd; }
		MojByte* data() { return m_begin; }
		MojByte* dataEnd() { return m_dataEnd; }

		void write(const void* data, MojSize size);
		void advance(MojSize size) { MojAssert(size <= freeSpace()); m_dataEnd += size; }

		void* operator new(MojSize objSize, MojSize bufSize) { return MojMalloc(objSize + bufSize); }
		void operator delete(void *obj) { MojFree(obj); }

	private:
		friend class MojBuffer;

		MojByte* m_begin;
		MojByte* m_end;
		MojByte* m_dataEnd;
		MojListEntry m_entry;
	};

	typedef MojVector<MojByte> ByteVec;

	MojBuffer();
	MojBuffer(MojBuffer& buf);
	~MojBuffer();

	bool empty() const;
	void iovec(MojIoVecT* vec, MojSize vecSize, MojSize& vecSizeOut) const;
	MojErr toByteVec(ByteVec& vecOut) const;

	void clear();
	MojErr writeByte(MojByte b);
	MojErr write(const void* data, MojSize size);
	MojErr data(const MojByte*& dataOut, MojSize& sizeOut);
	MojErr release(MojAutoPtr<Chunk>& chunkOut);
	void advance(MojSize size);

	MojBuffer& operator=(MojBuffer& rhs);

private:
	static const MojSize ChunkSize = 4096;
	typedef MojList<Chunk, &Chunk::m_entry> ChunkList;

	void pop();
	Chunk* allocChunk(MojSize size) const;
	MojErr writeableChunk(Chunk*& chunkOut, MojSize requestedSize);
	MojErr consolidate();

	ChunkList m_chunks;
	MojByte* m_readPos;
};

#endif /* MOJBUFFER_H_ */
