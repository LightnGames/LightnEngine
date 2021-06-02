#pragma once
#include <Core/Type.h>
#include <Core/CoreModuleSettings.h>
#include <Core/Math.h>
#include <string.h>
#include <stdio.h>

#define TARGET_WIN64 1

#if TARGET_WIN64
#include <Windows.h>
#define DEBUG_PRINT(x) OutputDebugString(x)
#endif

#ifdef LTN_DEBUG
#include <cassert>
#include <type_traits>
#define LTN_STATIC_ASSERT(x, s) static_assert(x, s)
#define LTN_ASSERT(x) assert(x)
#define LTN_INFO( str, ... )			 \
      {									 \
        char c[256];				 \
        sprintf_s(c, str, __VA_ARGS__ ); \
        DEBUG_PRINT(c);			 \
        DEBUG_PRINT("\n");		 \
      }

#define LTN_SUCCEEDED(hr) LTN_ASSERT(SUCCEEDED(hr))
#else
#define LTN_STATIC_ASSERT(x, s)
#define LTN_ASSERT(x) 
#define LTN_INFO( str, ... )
#define LTN_SUCCEEDED(hr) hr
#endif

#define LTN_COUNTOF(x) _countof(x)

constexpr char RESOURCE_FOLDER_PATH[] = "L:/LightnEngine/resource";
constexpr u32 FILE_PATH_COUNT_MAX = 128;
LTN_CORE_API u32 StrLength(const char* str);
LTN_CORE_API u64 StrHash(const char* str);
LTN_CORE_API u32 StrHash32(const char* str);
LTN_CORE_API u64 BinHash(const void* bin, u32 length);

constexpr u32 GetAligned(u32 size, u32 alignSize) {
	return size + (alignSize - (size % alignSize));
}

template <class T>
const T& Max(const T& a, const T& b) {
	return a < b ? b : a;
}

template <class T>
const T& Min(const T& a, const T& b) {
	return a > b ? b : a;
}

template <class T>
T RoundUp(const T& value, const T& divideValue) {
	return (value / divideValue) + 1;
}

class ThreeDigiets {
public:
	ThreeDigiets(u32 value) {
		char temp[64];
		sprintf_s(temp, "%d", value);

		u32 count = 0;
		for (u32 i = 0; i < LTN_COUNTOF(temp); ++i) {
			if (temp[i] == '\0') {
				count = i;
				break;
			}
		}

		u32 currentStrIndex = 0;
		u32 currentValue = value;
		for (u32 i = 0; i < count; ++i) {
			_str[currentStrIndex++] = temp[i];
			u32 strIndex = count - i - 1;
			bool isFirst = strIndex == 0;
			if (strIndex % 3 == 0 && !isFirst) {
				_str[currentStrIndex++] = ',';
			}
		}

		_str[currentStrIndex] = '\0';
	}

	const char* get() const { return _str; }

	char _str[64];
};

struct NonCopyable {
	NonCopyable() = default;
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;

	NonCopyable(NonCopyable&&) = default;
	NonCopyable& operator=(NonCopyable&&) = default;
};

class ChunkAllocator {
public:
	void doAlloc() {
		_dataPtr = new u8[_totalSizeInByte];
	}

	void terminate() {
		delete[] _dataPtr;
		_dataPtr = nullptr;
	}
private:
	u8* _dataPtr = nullptr;
	u32 _totalSizeInByte = 0;
};

template <class T, u32 N>
struct StaticArray {
	T _array[N];

	constexpr u32 getCount() { return N; }
	T& operator [](u32 index) { return _array[index]; }
	const T& operator [](u32 index) { return _array[index]; }
};

template <class T, u32 N>
struct LinerArray {
	T _array[N];
	u32 _count = 0;

	void push(T& value) {
		LTN_ASSERT(_count + 1 < N);
		_array[_count++] = value;
	}

	T* allocate(u32 numElements) {
		LTN_ASSERT(_count + numElements < N);
		T* currentPtr = _array[_count];
		_count += numElements;
		return currentPtr;
	}

	u32 getCount() { return _count; }
	constexpr u32 getCountMax() { return N; }
	T& operator [](u32 index) { return _array[index]; }
};

template <typename T, u32 U>
struct StaticQueue {
	void push(T data) {
		LTN_ASSERT(_instanceCount < U);
		_data[_queueEnd] = data;
		_instanceCount++;
		_queueEnd++;

		if (_queueEnd >= U) {
			_queueEnd = 0;
		}
	}

	T& front() {
		LTN_ASSERT(_instanceCount > 0);
		return _data[_queueStart];
	}

	void pop() {
		u32 queueStart = _queueStart;
		_instanceCount--;
		_queueStart++;
		if (_queueStart >= U) {
			_queueStart = 0;
		}
	}

	bool isEmpty() const { return _instanceCount == 0; }
	u32 getInstanceCount() const { return _instanceCount; }

private:
	u32 _queueStart = 0;
	u32 _queueEnd = 0;
	u32 _instanceCount = 0;
	T _data[U] = {};
};

class DynamicQueueController {
public:
	void initialize(u32 numElements) {
		cleanUp();

		_sizeCountMax = numElements;
		_emptyIndices = new u32[_sizeCountMax];
	}

	void terminate() {
		delete[] _emptyIndices;

		cleanUp();
	}

	u32 request() {
		LTN_ASSERT(_instanceCount < _sizeCountMax);
		if (_emptyIndicesCount > 0) {
			_emptyIndicesCount--;
			_instanceCount++;
			return _emptyIndices[_emptyIndicesCount];
		}

		u32 currentInstanceCount = _instanceCount;
		_instanceCount++;
		_resarveCount = Max(_resarveCount, _instanceCount);

		return currentInstanceCount;
	}

	void discard(u32 index) {
		_emptyIndices[_emptyIndicesCount] = index;
		_emptyIndicesCount++;
		_instanceCount--;
		LTN_ASSERT(_instanceCount != static_cast<u32>(-1));

		if (_instanceCount == 0) {
			resetEmptyIndices();
		}
	}

	u32 getInstanceCount() const { return _instanceCount; }
	u32 getResarveCount() const { return _resarveCount; }
	u32 getSizeCountMax() const { return _sizeCountMax; }

	void cleanUp() {
		_emptyIndices = nullptr;
		_instanceCount = 0;
		_resarveCount = 0;
		_sizeCountMax = 0;
		_emptyIndicesCount = 0;
	}

	void resetEmptyIndices() {
		_emptyIndicesCount = 0;
		memset(_emptyIndices, 0, sizeof(u32) * _sizeCountMax);
	}

public:
	u32* _emptyIndices = nullptr;
	u32 _instanceCount = 0;
	u32 _resarveCount = 0;
	u32 _sizeCountMax = 0;
	u32 _emptyIndicesCount = 0;
};

template <class T>
class DynamicArray {
public:
	void initialize(u32 numElements) {
		_array = new T[numElements]();
		_elementCount = numElements;
	}

	void resize(u32 numElements) {
		T* newArray = new T[numElements];
		for (u32 i = 0; i < _elementCount; ++i) {
			newArray[i] = _array[i];
		}
		terminate();
		_array = newArray;
		_elementCount = numElements;
	}

	void terminate() {
		delete[] _array;
		_array = nullptr;
		_elementCount = 0;
	}

	T& operator [](u32 index) { return _array[index]; }
	const T& operator [](u32 index) const { return _array[index]; }

private:
	T* _array = nullptr;
	u32 _elementCount = 0;
};

template <class T>
class DynamicQueue {
public:
	void initialize(u32 numElements) {
		_data = new T[numElements]();
		_controller.initialize(numElements);
	}

	void terminate() {
		delete[] _data;
		_controller.terminate();
	}

	u32 request() {
		return _controller.request();
	}

	void discard(u32 index) {
		_data[index] = T();
		_controller.discard(index);
	}

	u32 getArrayIndex(const T* data) const { return static_cast<u32>(data - _data); }

	u32 getInstanceCount() const { return _controller.getInstanceCount(); }
	u32 getResarveCount() const { return _controller.getResarveCount(); }
	u32 getSizeCountMax() const { return _controller.getSizeCountMax(); }

	T& operator [](u32 index) { return _data[index]; }
	const T& operator [](u32 index) const { return _data[index]; }

public:
	T* _data = nullptr;
	DynamicQueueController _controller;
};

class ValueDynamicQueue {
public:
	void initialize(u32 valueSizeInByte, u32 numElements) {
		_valueSizeInByte = valueSizeInByte;
		_data = new u8[numElements]();
		_controller.initialize(numElements);
	}

	void terminate() {
		delete[] _data;
		_controller.terminate();
	}

	u32 request() {
		return _controller.request();
	}

	void discard(u32 index) {
		memset(&_data[index * _valueSizeInByte], 0, _valueSizeInByte);
		_controller.discard(index);
	}

	template <class T>
	u32 getArrayIndex(const T* data) const { return static_cast<u32>(data - reinterpret_cast<T*>(_data)); }

	u32 getInstanceCount() const { return _controller.getInstanceCount(); }
	u32 getResarveCount() const { return _controller.getResarveCount(); }
	u32 getSizeCountMax() const { return _controller.getSizeCountMax(); }

	template <class T>
	T& operator [](u32 index) { return reinterpret_cast<T*>(_data)[index]; }

	template <class T>
	const T& operator [](u32 index) const { return reinterpret_cast<T*>(_data)[index]; }

public:
	u8* _data = nullptr;
	u32 _valueSizeInByte = 0;
	DynamicQueueController _controller;
};


class LTN_CORE_API MultiDynamicQueueBlockManager {
public:
	static constexpr u32 INVILD_BLOCK_INDEX = 0xffffffff;

	struct EmptyBlockHeader {
		u32 _index = 0;
		u32 _size = 0;
	};

	void initialize(u32 numElements) {
		_emptyBlockInfo.initialize(numElements);

		// 最初に空の最大サイズブロックを追加
		u32 firstBlockIndex = _emptyBlockInfo.request();
		EmptyBlockHeader& firstBlock = _emptyBlockInfo[firstBlockIndex];
		firstBlock._index = 0;
		firstBlock._size = numElements;
	}

	void terminate() {
		_emptyBlockInfo.terminate();
	}

	bool isEmpty() const {
		return _instanceCount == 0 && _emptyBlockInfo.getInstanceCount() <= 1;
	}

	u32 getInstanceCount() const { return _instanceCount; }

	u32 request(u32 numElements) {
		u32 bestBlockIndex = INVILD_BLOCK_INDEX;
		u32 bestBlockSize = static_cast<u32>(-1);
		u32 emptyBlockInfoCount = _emptyBlockInfo.getResarveCount();

		// もっともサイズが近い空のブロックを検索
		for (u32 blockIndex = 0; blockIndex < emptyBlockInfoCount; ++blockIndex) {
			const EmptyBlockHeader& blockHeader = _emptyBlockInfo[blockIndex];
			if (blockHeader._size < numElements) {
				continue;
			}

			if (blockHeader._size < bestBlockSize) {
				bestBlockIndex = blockIndex;
				bestBlockSize = blockHeader._size;
			}
		}

		// ブロックが見つからない
		LTN_ASSERT(bestBlockIndex != INVILD_BLOCK_INDEX);

		EmptyBlockHeader& bestBlockHeader = _emptyBlockInfo[bestBlockIndex];
		LTN_ASSERT(bestBlockHeader._size >= numElements);

		u32 currentIndex = bestBlockHeader._index;
		bestBlockHeader._index += numElements;
		bestBlockHeader._size -= numElements;

		// ベストフィットブロックなら空ブロックリストから削除
		if (bestBlockHeader._size == 0) {
			_emptyBlockInfo.discard(bestBlockIndex);
		}

		_instanceCount += numElements;
		return currentIndex;
	}

	void discard(u32 dataIndex, u32 numElements) {
		u32 discardHeaderIndex = _emptyBlockInfo.request();
		EmptyBlockHeader& discardHeader = _emptyBlockInfo[discardHeaderIndex];
		discardHeader._index = dataIndex;
		discardHeader._size = numElements;

		// 返却されたブロックをマージする
		u32 mergedHeaderIndex = mergePrev(discardHeaderIndex);
		mergeNext(mergedHeaderIndex);
		_instanceCount -= numElements;
	}

private:
	// 前方ブロックマージ
	u32 mergePrev(u32 baseBlockIndex) {
		const EmptyBlockHeader& baseHeader = _emptyBlockInfo[baseBlockIndex];
		u32 dataIndex = baseHeader._index;
		u32 mergedHeaderIndex = baseBlockIndex;
		u32 dataCount = baseHeader._size;

		u32 emptyBlockHeaderCount = _emptyBlockInfo.getResarveCount();
		for (u32 blockIndex = 0; blockIndex < emptyBlockHeaderCount; ++blockIndex) {
			const EmptyBlockHeader& prevBlockHeader = _emptyBlockInfo[blockIndex];
			if (prevBlockHeader._size == 0) {
				continue;
			}

			if (blockIndex == baseBlockIndex) {
				continue;
			}

			u32 prevBlockIndex = dataIndex - prevBlockHeader._size;
			if (prevBlockIndex == prevBlockHeader._index) {
				EmptyBlockHeader& mergeBlockHeader = _emptyBlockInfo[blockIndex];
				mergeBlockHeader._size += dataCount;
				mergedHeaderIndex = blockIndex;

				_emptyBlockInfo.discard(baseBlockIndex);
				break;
			}
		}

		return mergedHeaderIndex;
	}

	// 後方ブロックマージ
	void mergeNext(u32 baseBlockIndex) {
		const EmptyBlockHeader& baseHeader = _emptyBlockInfo[baseBlockIndex];
		u32 dataIndex = baseHeader._index + baseHeader._size;
		u32 emptyBlockHeaderCount = _emptyBlockInfo.getResarveCount();
		for (u32 nextBlockIndex = 0; nextBlockIndex < emptyBlockHeaderCount; ++nextBlockIndex) {
			const EmptyBlockHeader& nextBlockHeader = _emptyBlockInfo[nextBlockIndex];
			if (nextBlockHeader._size == 0) {
				continue;
			}

			if (nextBlockIndex == baseBlockIndex) {
				continue;
			}

			if (dataIndex == nextBlockHeader._index) {
				EmptyBlockHeader& mergeBlockHeader = _emptyBlockInfo[baseBlockIndex];
				mergeBlockHeader._size += nextBlockHeader._size;

				_emptyBlockInfo.discard(nextBlockIndex);
				break;
			}
		}
	}

private:
	u32 _instanceCount = 0;
	DynamicQueue<EmptyBlockHeader> _emptyBlockInfo;
};

template <class T>
class MultiDynamicQueue {
public:
	static constexpr u32 INVILD_INDEX = 0xffffffff;

	void initialize(u32 numElements) {
		_data = new T[numElements]();
		_blockManager.initialize(numElements);
	}
	void terminate() {
		delete[] _data;
		_blockManager.terminate();
	}

	u32 request(u32 numElements) {
		_instanceCount += numElements;
		_resarveCount = Max(_instanceCount, _resarveCount);
		u32 index = _blockManager.request(numElements);
		LTN_ASSERT(index != INVILD_INDEX);
		return index;
	}

	u32 requestIndex(T* data) {
		return static_cast<u32>(data - _data);
	}

	u32 getInstanceCount() const {
		return _instanceCount;
	}

	u32 getResarveCount() const {
		return _resarveCount;
	}

	void discard(T* data, u32 numElements) {
		LTN_ASSERT(_instanceCount - numElements < _resarveCount);
		_instanceCount -= numElements;
		u32 dataIndex = static_cast<u32>(data - _data);
		_blockManager.discard(dataIndex, numElements);
	}

	T& operator [](u32 index) { return _data[index]; }
	const T& operator [](u32 index) const { return _data[index]; }

private:
	T* _data = nullptr;
	u32 _instanceCount = 0;
	u32 _resarveCount = 0;
	MultiDynamicQueueBlockManager _blockManager;
};

template<class T>
class LinerAllocater {
public:
	void initialize(u32 numElements) {
		_dataPtr = new T[numElements]();
		_numElements = numElements;
	}

	void terminate() {
		delete[] _dataPtr;
		_dataPtr = nullptr;
		_numElements = 0;
		_allocatedNumElements = 0;
	}

	T* allocate(u32 numElements = 1) {
		LTN_ASSERT(_allocatedNumElements + numElements < _numElements);
		u32 currentSizeInByte = _allocatedNumElements;
		_allocatedNumElements += numElements;
		return _dataPtr + currentSizeInByte;
	}

	T* get(u32 index = 0) {
		return &_dataPtr[index];
	}

	u32 getCount() const {
		return _allocatedNumElements;
	}

	void reset() {
		_allocatedNumElements = 0;
	}

	T* _dataPtr = nullptr;
	u32 _numElements = 0;
	u32 _allocatedNumElements = 0;
};

struct AABB {
	AABB() {}
	AABB(const Vector3& min, const Vector3& max) :_min(min), _max(max) {}

	AABB getTransformedAabb(const Matrix4& matrix) {
		Vector3 min = _min;
		Vector3 size = getSize();

		const Vector3 points[8] = {
			min + Vector3(size._x,size._y,size._z),
			min + Vector3(size._x,size._y,0),
			min + Vector3(size._x,0,0),
			min + Vector3(0,0,0),
			min + Vector3(size._x,0,size._z),
			min + Vector3(0,0,size._z),
			min + Vector3(0,size._y,size._z),
			min + Vector3(0,size._y,0), };

		Vector3 resultMin = Vector3::One * 100000;
		Vector3 resultMax = Vector3::One * -100000;

		for (int i = 0; i < 8; ++i) {
			Vector3 p = Matrix4::transform(points[i], matrix);
			resultMin = Vector3::Min(p, resultMin);
			resultMax = Vector3::Max(p, resultMax);
		}

		return AABB(resultMin, resultMax);
	}

	Vector3 getSize() const {
		return (_max - _min);
	}

	Vector3 getHalfSize() const {
		return getSize() / 2.0f;
	}

	Vector3 getCenter() const {
		return _min + getHalfSize();
	}

	Vector3 _min;
	Vector3 _max;
};