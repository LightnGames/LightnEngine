#pragma once

#include <memory>

template <typename T>
class RefPtr {

public:

	RefPtr():_ptr{nullptr}{}
	RefPtr(T* src):_ptr{src}{}

	//����ɔj���E�|�C���^�̐����֎~
	//~RefPtr() = delete;
	void* operator new(std::size_t s) = delete;
	//void operator =(const T& data) = delete;

	//�ێ����Ă���|�C���^��Ԃ�
	T* operator ->() const noexcept { return _ptr; }

	//�|�C���^�f�[�^���擾
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