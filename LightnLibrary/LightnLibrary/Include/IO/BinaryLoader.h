#pragma once

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <memory>

class BinaryLoader {

public:

	BinaryLoader(const LPCSTR& fileName) {
		//�t�@�C���I�[�v��
		std::fstream file(fileName, std::ios::in | std::ios::binary);

		//�ǂݍ��ݎ��̊m�F
		if (!file) {
			return;
		}

		//�t�@�C���|�C���^�̐擪
		int begin = static_cast<int>(file.tellg());
		file.seekg(0, std::ios::end);

		//�t�@�C���|�C���^�̖���
		int end = static_cast<int>(file.tellg());

		//�������擾
		int length = end - begin;
		file.clear();
		file.seekg(0, file.beg);

		//Chara�|�C���^�Ƀo�C�i���f�[�^��ǂݍ���
		std::unique_ptr<char> buffer(new char[length]);
		file.read(buffer.get(), length);
		file.close();

		_length = length;
		_bin = std::move(buffer);
	}

	//�o�C�i���f�[�^���擾
	void* data() {
		return _bin.get();
	}

	//�o�C�i���f�[�^�̒������擾
	int size() {
		return _length;
	}

private:

	int _length;
	std::unique_ptr<char> _bin;

};