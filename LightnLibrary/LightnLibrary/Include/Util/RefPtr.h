#pragma once

#include <memory>

template <typename T>
class RefPtr {

public:

	RefPtr():_ptr{nullptr}{}
	RefPtr(T* src):_ptr{src}{}

	//勝手に破棄・ポインタの生成禁止
	//~RefPtr() = delete;
	void* operator new(std::size_t s) = delete;
	//void operator =(const T& data) = delete;

	//保持しているポインタを返す
	T* operator ->() const noexcept { return _ptr; }

	//ポインタデータを取得
	const T* get() const noexcept { return _ptr; }

	template <typename NT>
	RefPtr<NT> cast() {
		return RefPtr<NT>((NT*)_ptr);
	}

	bool operator ==(const RefPtr<T>& other) {
		return other._ptr == _ptr;
	}

private:

	T * _ptr;

};