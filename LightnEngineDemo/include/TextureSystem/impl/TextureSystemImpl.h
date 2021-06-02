#pragma once
#include <TextureSystem/TextureSystem.h>
#include <GfxCore/impl/GpuResourceImpl.h>

enum TextureStateFlags {
	TEXTURE_STATE_NONE = 0,
	TEXTURE_STATE_ENABLE,
	TEXTURE_STATE_REQUEST_DELETE,
};

struct TextureDebugName {
	char _name[FILE_PATH_COUNT_MAX] = {};
};

class TextureImpl :public Texture {
public:
	virtual void requestToDelete() override {
		*_stateFlags |= TEXTURE_STATE_REQUEST_DELETE;
	}
	void initialize(const TextureDesc& desc);
	void terminate();
	void setStateFlags(u8* flags) {
		_stateFlags = flags;
	}
	void setGpuTexture(GpuTexture* texture) {
		_texture = texture;
	}
	void setAsset(Asset* asset) {
		_asset = asset;
	}
	GpuTexture* getGpuTexture() {
		return _texture;
	}
private:
	u8* _stateFlags = nullptr;
	GpuTexture* _texture = nullptr;
};

class LTN_TEXTURE_SYSTEM_API TextureSystemImpl :public TextureSystem {
public:
	static constexpr u32 TEXTURE_COUNT_MAX = 512;
	void initialize();
	void update();
	void processDeletion();
	void terminate();
	void debugDrawGui();
	void loadTexture(u32 textureIndex);

	u32 getTextureIndex(const Texture* texture) const;
	Texture* findTexture(u64 fileHash);
	DescriptorHandle getDescriptors() const { return _descriptors; }
	virtual Texture* createTexture(const TextureDesc& desc) override;
	virtual Texture* allocateTexture(const TextureDesc& desc) override;

	static TextureSystemImpl* Get();
private:
	u32 findTextureFileHash(u64 hash);

private:
	DynamicQueue<TextureImpl> _textures;
	GpuTexture _gpuTextures[TEXTURE_COUNT_MAX] = {};
	TextureDebugName _debugNames[TEXTURE_COUNT_MAX] = {};
	Asset* _textureAssets = nullptr;
	u8 _assetStateFlags[TEXTURE_COUNT_MAX] = {};

	u64 _fileHashes[TEXTURE_COUNT_MAX] = {};
	u8 _stateFlags[TEXTURE_COUNT_MAX] = {};
	DescriptorHandle _descriptors;
	Texture* _commonBlackTexture = nullptr;
};