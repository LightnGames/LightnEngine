#pragma once


template <class T>
class ComPtr {

public:

	explicit ComPtr(T* pInterface = nullptr, bool addition = false) {
		//�|�C���^�����L����ꍇ�͎Q�ƃJ�E���^�𑝉�
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

	//�R�s�[�R���X�g���N�^
	ComPtr(const ComPtr& src) throw() {
		//�R�s�[�������łɃC���^�[�t�F�[�X�������Ă���ꍇ�͋��L���ǉ������̂ŎQ�ƃJ�E���^�𑝉�
		if (src._pInterface != nullptr) {
			src._pInterface->AddRef();
		}
		_pInterface = src._pInterface;
	}

	//�|�C���^���擾
	T* Get() {
		return _pInterface;
	}

	//�_�u���|�C���^���擾
	T** GetAddressOf() {
		return &_pInterface;
	}

	//�Q�ƃJ�E���^�����炵�ă_�u���|�C���^���擾(�_�u���|�C���^��n���ď���������ۂȂǂɎg�p)
	T** ReleaseAndGetAddressOf(){
		if (_pInterface) {
			_pInterface->Release();
		}

		return &_pInterface;
	}

	ComPtr& operator = (const ComPtr& src) throw() {
		//�R�s�[���̎Q�ƃJ�E���^�𑝉�
		if (src._pInterface) {
			src._pInterface->AddRef();
		}

		//�����̎Q�ƃJ�E���^������
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