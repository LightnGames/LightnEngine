#pragma once
#include <Core/System.h>
#include <cstdio>

enum AssetStateFlags {
	ASSET_STATE_NONE = 0,
	ASSET_STATE_ALLOCATED,
	ASSET_STATE_REQUEST_LOAD,
	ASSET_STATE_LOADING,
	ASSET_STATE_ENABLE,
	ASSET_STATE_UNLOADING
};

class Asset {
public:
	void requestLoad() {
		LTN_ASSERT(*_assetStateFlags == ASSET_STATE_ALLOCATED);
		*_assetStateFlags = ASSET_STATE_REQUEST_LOAD; 
	}
	void setAssetStateFlags(u8* flags) { _assetStateFlags = flags; }
	void openFile(const char* filePath) {
		s32 error = fopen_s(&_file, filePath, "rb");
		LTN_ASSERT(_file != nullptr);
		sprintf_s(_filePath, LTN_COUNTOF(_filePath), "%s", filePath);
	}

	void closeFile() {
		fclose(_file);
		_file = nullptr;
	}

	void updateCurrentSeekPosition() {
		fpos_t size = 0;
		fgetpos(_file, &size);
		_fileOffsetInByte = static_cast<u32>(size);
	}
	FILE* getFile() { return _file; }
	u32 getFileOffsetInByte() const { return _fileOffsetInByte; }
	const char* getFilePath() const { return _filePath; }

protected:
	FILE* _file = nullptr;
	char _filePath[256];
	u32 _fileOffsetInByte = 0;
	u8* _assetStateFlags = nullptr;
};