#pragma once

#include <Util/RefPtr.h>
#include <cassert>

template <typename T>
class Singleton {

public:

	Singleton() {
		assert(!mSingleton);
		mSingleton = static_cast<T*>(this);
	}

	static T& instance() {
		return *mSingleton;
	}

	void operator =(const T& src) = delete;

protected:

	static T* mSingleton;
};