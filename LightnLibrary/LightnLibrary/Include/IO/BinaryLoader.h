#pragma once

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <memory>

class BinaryLoader {

public:

	BinaryLoader(const LPCSTR& fileName) {
		//ファイルオープン
		std::fstream file(fileName, std::ios::in | std::ios::binary);

		//読み込み事故確認
		if (!file) {
			return;
		}

		//ファイルポインタの先頭
		int begin = static_cast<int>(file.tellg());
		file.seekg(0, std::ios::end);

		//ファイルポインタの末尾
		int end = static_cast<int>(file.tellg());

		//長さを取得
		int length = end - begin;
		file.clear();
		file.seekg(0, file.beg);

		//Charaポインタにバイナリデータを読み込み
		std::unique_ptr<char> buffer(new char[length]);
		file.read(buffer.get(), length);
		file.close();

		length_ = length;
		bin_ = std::move(buffer);
	}

	//バイナリデータを取得
	void* data() {
		return bin_.get();
	}

	//バイナリデータの長さを取得
	int size() {
		return length_;
	}

private:

	int length_;
	std::unique_ptr<char> bin_;

};