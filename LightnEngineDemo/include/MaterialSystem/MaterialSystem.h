#pragma once
#include <MaterialSystem/ModuleSettings.h>
#include <Core/System.h>
#include <Core/Asset.h>

class Texture;

struct LTN_MATERIAL_SYSTEM_API Material {
	static constexpr u16 INVALID_PARAMETER_INDEX = static_cast<u16>(-1);
	static constexpr u16 PARAM_TYPE_SIZE_IN_BYTE[] = { 4,8,12,16,4,4 };
	enum ShaderValiableType {
		FLOAT = 0,
		FLOAT2,
		FLOAT3,
		FLOAT4,
		UINT,
		TEXTURE,
		COUNT
	};

	virtual void requestToDelete() = 0;
	virtual void setTexture(u32 nameHash, Texture* texture) = 0;
	virtual const u8* getParameterRaw(u32 nameHash) const = 0;

	template<class T>
	const T* getParameter(u32 nameHash) const {
		return reinterpret_cast<const T*>(getParameterRaw(nameHash));
	}

	template<class T>
	void setParameter(u32 nameHash, const T& data) {
		setParameterRaw(nameHash, &data);
	}

protected:
	virtual void setParameterRaw(u32 nameHash, const void* dataPtr) = 0;

protected:
	u8* _stateFlags = nullptr;
	u8* _updateFlags = nullptr;
};

struct LTN_MATERIAL_SYSTEM_API ShaderSet {
	virtual void requestToDelete() = 0;
	virtual void setTexture(Texture* texture, u64 parameterNameHash) = 0;

protected:
	u8* _stateFlags = nullptr;
	u8* _updateFlags = nullptr;
};

struct MaterialDesc {
	const char* _filePath = nullptr;
};

struct ShaderSetDesc {
	const char* _filePath = nullptr;
};

class LTN_MATERIAL_SYSTEM_API MaterialSystem {
public:
	static constexpr u32 MATERIAL_COUNT_MAX = 256;
	static constexpr u32 SHADER_SET_COUNT_MAX = 32;

	virtual ShaderSet* createShaderSet(const ShaderSetDesc& desc) = 0;
	virtual Material* createMaterial(const MaterialDesc& desc) = 0;
	virtual Material* findMaterial(u64 filePathHash) = 0;

	static MaterialSystem* Get();
};