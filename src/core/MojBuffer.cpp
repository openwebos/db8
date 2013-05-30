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


#include "core/MojBuffer.h"

MojBuffer::Chunk::Chunk(gsize size)
: m_begin((guint8*) (this + 1)),
  m_end(m_begin + size),
  m_dataEnd(m_begin)
{
}

void MojBuffer::Chunk::write(const void* data, gsize size)
{
	MojAssert(size <= freeSpace());
	MojMemCpy(m_dataEnd, data, size);
	m_dataEnd += size;
}

MojBuffer::MojBuffer()
: m_readPos(NULL)
{
}

MojBuffer::MojBuffer(MojBuffer& buf)
: m_chunks(buf.m_chunks),
  m_readPos(buf.m_readPos)
{
	buf.m_readPos = NULL;
}

MojBuffer::~MojBuffer()
{
	clear();
	MojAssert(m_chunks.empty());
}

bool MojBuffer::empty() const
{
	return m_readPos == NULL ||
			(m_chunks.size() == 1 && m_readPos == m_chunks.front()->dataEnd());
}

void MojBuffer::iovec(MojIoVecT* vec, gsize vecSize, gsize& vecSizeOut) const
{
	vecSizeOut = 0;
	ChunkList::ConstIterator i = m_chunks.begin();
	while (vecSizeOut < vecSize && i != m_chunks.end()) {
		const guint8* base = (i == m_chunks.begin()) ? m_readPos : (*i)->data();
		gsize len = (*i)->dataEnd() - base;
		if (len > 0) {
			vec[vecSizeOut].iov_base = (void*) base;
			vec[vecSizeOut].iov_len = len;
			++vecSizeOut;
		}
		++i;
	}
}

MojErr MojBuffer::toByteVec(ByteVec& vecOut) const
{
	vecOut.clear();
	for (ChunkList::ConstIterator i = m_chunks.begin(); i != m_chunks.end(); ++i) {
		const guint8* base = (i == m_chunks.begin()) ? m_readPos : (*i)->data();
		MojErr err = vecOut.append(base, base + (*i)->dataSize());
		MojErrCheck(err);
	}
	return MojErrNone;
}

void MojBuffer::clear()
{
	while (!m_chunks.empty()) {
		pop();
	}
}

MojErr MojBuffer::writeByte(guint8 b)
{
	Chunk* chunk;
	MojErr err = writeableChunk(chunk, 1);
	MojErrCheck(err);
	*chunk->dataEnd() = b;
	chunk->advance(1);

	return MojErrNone;
}

MojErr MojBuffer::write(const void* data, gsize size)
{
	MojAssert(data || size == 0);

	const guint8* bytes = static_cast<const guint8*>(data);
	while (size > 0) {
		Chunk* chunk;
		MojErr err = writeableChunk(chunk, size);
		MojErrCheck(err);
		gsize chunkSize = MojMin(size, chunk->freeSpace());
		chunk->write(bytes, chunkSize);
		bytes += chunkSize;
		size -= chunkSize;
	}
	return MojErrNone;
}

MojErr MojBuffer::data(const guint8*& dataOut, gsize& sizeOut)
{
	dataOut = NULL;
	sizeOut = 0;
	MojErr err = consolidate();
	MojErrCheck(err);
	if (m_chunks.size() == 1) {
		// if we only have one chunk, just return it's data
		const Chunk* chunk = m_chunks.front();
		dataOut = chunk->data();
		sizeOut = chunk->dataSize();
	}
	return MojErrNone;
}

MojErr MojBuffer::release(MojAutoPtr<Chunk>& chunkOut)
{
	MojErr err = consolidate();
	MojErrCheck(err);
	MojAssert(m_chunks.size() <= 1);
	if (m_chunks.empty()) {
		chunkOut.reset();
	} else {
		chunkOut.reset(m_chunks.popFront());
	}
	return MojErrNone;
}

void MojBuffer::advance(gsize size)
{
	while (size > 0) {
		MojAssert(!m_chunks.empty());
		const Chunk* chunk = m_chunks.front();
		MojAssert(m_readPos >= chunk->data() && m_readPos <= chunk->dataEnd());
		gsize advanceSize = MojMin(size, (gsize) (chunk->dataEnd() - m_readPos));
		size -= advanceSize;
		m_readPos += advanceSize;
		if (size > 0)
			pop();
	}
}

MojBuffer& MojBuffer::operator=(MojBuffer& rhs)
{
	m_chunks = rhs.m_chunks;
	m_readPos = rhs.m_readPos;
	rhs.m_readPos = NULL;
	return *this;
}

void MojBuffer::pop()
{
	MojAssert(!m_chunks.empty());
	delete m_chunks.popFront();
	if (!m_chunks.empty()) {
		m_readPos = m_chunks.front()->data();
	} else {
		m_readPos = NULL;
	}
}

MojBuffer::Chunk* MojBuffer::allocChunk(gsize size) const
{
	return new(size) Chunk(size);
}

MojErr MojBuffer::writeableChunk(Chunk*& chunkOut, gsize requestedSize)
{
	if (!m_chunks.empty()) {
		chunkOut = m_chunks.back();
		if (chunkOut->freeSpace() > 0)
			return MojErrNone;
	}
	chunkOut = allocChunk(MojMax(requestedSize, ChunkSize - sizeof(Chunk)));
	MojAllocCheck(chunkOut);
	m_chunks.pushBack(chunkOut);
	if (m_readPos == NULL)
		m_readPos = chunkOut->data();

	return MojErrNone;
}

MojErr MojBuffer::consolidate()
{
	if (m_chunks.size() > 1) {
		// calc total size
		gsize size = 0;
		for (ChunkList::ConstIterator i = m_chunks.begin(); i != m_chunks.end(); ++i) {
			size += (*i)->dataSize();
		}
		// alloc chunk big enoug to hold it all
		Chunk* chunk = allocChunk(size);
		MojAllocCheck(chunk);
		// copy alldata to new chunk
		for (ChunkList::ConstIterator i = m_chunks.begin(); i != m_chunks.end(); ++i) {
			chunk->write((*i)->data(), (*i)->dataSize());
		}
		// replace existing chunks with new one
		clear();
		m_chunks.pushBack(chunk);
	}
	return MojErrNone;
}
