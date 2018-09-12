#pragma once


template <class T>
class ComPtr {

public:

	explicit ComPtr(T* pInterface = nullptr, bool addition = false) {
		//ポインタを共有する場合は参照カウンタを増加
		if (pInterface && addition) {
			pInterface->Release();
		}
		_pInterface = pInterface;
	}

	~ComPtr() {
		if (_pInterface != nullptr) {
			_pInterface->Release();
		}
	}

	//コピーコンストラクタ
	ComPtr(const ComPtr& src) throw() {
		//コピー元がすでにインターフェースを持っている場合は共有が追加されるので参照カウンタを増加
		if (src._pInterface != nullptr) {
			src._pInterface->AddRef();
		}
		_pInterface = src._pInterface;
	}

	//ポインタを取得
	T* Get() {
		return _pInterface;
	}

	//ダブルポインタを取得
	T** GetAddressOf() {
		return &_pInterface;
	}

	//参照カウンタを減らしてダブルポインタを取得(ダブルポインタを渡して初期化する際などに使用)
	T** ReleaseAndGetAddressOf(){
		if (_pInterface) {
			_pInterface->Release();
		}

		return &_pInterface;
	}

	ComPtr& operator = (const ComPtr& src) throw() {
		//コピー元の参照カウンタを増加
		if (src._pInterface) {
			src._pInterface->AddRef();
		}

		//自分の参照カウンタを減少
		if (_pInterface) {
			_pInterface->Release();
		}

		_pInterface = src._pInterface;

		return *this;
	}

	T* operator ->() {
		return _pInterface;
	}

	bool operator ==(const T* value) {
		return (value == _pInterface);
	}

	bool operator !=(const T* value) {
		return (value != _pInterface);
	}

public:

	T * _pInterface;
};