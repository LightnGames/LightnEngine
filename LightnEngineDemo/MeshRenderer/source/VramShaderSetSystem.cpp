#include <MeshRenderer/impl/VramShaderSetSystem.h>
#include <GfxCore/impl/GraphicsSystemImpl.h>
#include <GfxCore/impl/QueryHeapSystem.h>
#include <MeshRenderer/GpuStruct.h>
#include <MaterialSystem/impl/PipelineStateSystem.h>
#include <MaterialSystem/impl/MaterialSystemImpl.h>
#include <TextureSystem/impl/TextureSystemImpl.h>

void VramShaderSetSystem::initialize() {
	Device* device = GraphicsSystemImpl::Get()->getDevice();
	DescriptorHeapAllocator* allocator = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	{
		GpuBufferDesc desc = {};
		desc._sizeInByte = MaterialSystem::MATERIAL_COUNT_MAX * sizeof(u32);
		desc._initialState = RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		desc._device = device;
		_materialInstanceIndexBuffer.initialize(desc);
		_materialInstanceIndexBuffer.setDebugName("Material Instance Index");
	}

	{
		ShaderResourceViewDesc desc = {};
		desc._format = FORMAT_R32_TYPELESS;
		desc._viewDimension = SRV_DIMENSION_BUFFER;
		desc._buffer._firstElement = 0;
		desc._buffer._flags = BUFFER_SRV_FLAG_RAW;
		desc._buffer._numElements = MaterialSystem::MATERIAL_COUNT_MAX;
		_materialInstanceIndexSrv = allocator->allocateDescriptors(1);
		device->createShaderResourceView(_materialInstanceIndexBuffer.getResource(), &desc, _materialInstanceIndexSrv._cpuHandle);
	}
}

void VramShaderSetSystem::update() {
	VramBufferUpdater* vramUpdater = GraphicsSystemImpl::Get()->getVramUpdater();
	MaterialSystemImpl* materialSystem = MaterialSystemImpl::Get();
	MaterialImpl* materials = materialSystem->getMaterial();
	const u8* materialUpdateFlags = materialSystem->getMaterialUpdateFlags();
	const u8* materialStateFlags = materialSystem->getMaterialStateFlags();
	const u8* shaderSetStateFlags = materialSystem->getShaderSetStateFlags();
	u32 updateMaterialCount = 0;
	u32 materialCount = materialSystem->getMaterialCount();
	u32 shaderSetCount = materialSystem->getShaderSetCount();
	for (u32 shaderSetIndex = 0; shaderSetIndex < shaderSetCount; ++shaderSetIndex) {
		if (shaderSetStateFlags[shaderSetIndex] & SHADER_SET_STATE_FLAG_CREATED) {
			_shaderSets[shaderSetIndex].initialize();
		}
	}

	for (u32 materialIndex = 0; materialIndex < materialCount; ++materialIndex) {
		if (materialStateFlags[materialIndex] & MATERIAL_STATE_FLAG_CREATED) {
			MaterialImpl* material = &materials[materialIndex];
			u32 shaderSetIndex = materialSystem->getShaderSetIndex(material->getShaderSet());
			++updateMaterialCount;

			VramShaderSet& shaderSet = _shaderSets[shaderSetIndex];
			u32 materialInstanceIndex = shaderSet.findMaterialInstance(material);
			if (materialInstanceIndex == gpu::INVALID_INDEX) {
				materialInstanceIndex = shaderSet.addMaterialInstance(material);

				u32 materialIndex = materialSystem->getMaterialIndex(material);
				_materialInstanceIndices[materialIndex] = materialInstanceIndex;
				_shaderSetIndices[materialIndex] = shaderSetIndex;
			}
		}
	}

	for (u32 materialIndex = 0; materialIndex < materialCount; ++materialIndex) {
		if (materialUpdateFlags[materialIndex] & MATERIAL_UPDATE_FLAG_UPDATE_PARAMS) {
			u32 shaderSetIndex = _shaderSetIndices[materialIndex];
			u32 materialInstanceIndex = _materialInstanceIndices[materialIndex];
			_shaderSets[shaderSetIndex].updateMaterialParameter(materialInstanceIndex);
		}
	}

	if (updateMaterialCount > 0) {
		u32* mapIndices = vramUpdater->enqueueUpdate<u32>(&_materialInstanceIndexBuffer, 0, MaterialSystem::MATERIAL_COUNT_MAX);
		memcpy(mapIndices, _materialInstanceIndices, MaterialSystem::MATERIAL_COUNT_MAX * sizeof(u32));
	}
}

void VramShaderSetSystem::processDeletion() {
	VramBufferUpdater* vramUpdater = GraphicsSystemImpl::Get()->getVramUpdater();
	MaterialSystemImpl* materialSystem = MaterialSystemImpl::Get();
	MaterialImpl* materials = materialSystem->getMaterial();
	const u8* materialStateFlags = materialSystem->getMaterialStateFlags();
	u32 materialCount = materialSystem->getMaterialCount();
	const u8* shaderSetStateFlags = materialSystem->getShaderSetStateFlags();
	u32 shaderSetCount = materialSystem->getShaderSetCount();
	for (u32 materialIndex = 0; materialIndex < materialCount; ++materialIndex) {
		if (materialStateFlags[materialIndex] & MATERIAL_STATE_FLAG_REQEST_DESTROY) {
			MaterialImpl* material = &materials[materialIndex];
			u32 shaderSetIndex = _shaderSetIndices[materialIndex];
			u32 materialInstanceIndex = _materialInstanceIndices[materialIndex];
			_shaderSets[shaderSetIndex].removeMaterialInstance(materialInstanceIndex);
		}
	}

	for (u32 shaderSetIndex = 0; shaderSetIndex < shaderSetCount; ++shaderSetIndex) {
		if (shaderSetStateFlags[shaderSetIndex] & SHADER_SET_STATE_FLAG_REQEST_DESTROY) {
			_shaderSets[shaderSetIndex].terminate();
			_shaderSets[shaderSetIndex] = VramShaderSet();
		}
	}
}

void VramShaderSetSystem::terminate() {
	_materialInstanceIndexBuffer.terminate();

	DescriptorHeapAllocator* allocator = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	allocator->discardDescriptor(_materialInstanceIndexSrv);
}

u32 VramShaderSetSystem::getShaderSetIndex(const Material* material) {
	MaterialSystemImpl* materialSystem = MaterialSystemImpl::Get();
	return materialSystem->getShaderSetIndex(static_cast<const MaterialImpl*>(material)->getShaderSet());
}

void VramShaderSet::initialize() {
	Device* device = GraphicsSystemImpl::Get()->getDevice();

	// buffers
	{
		GpuBufferDesc desc = {};
		desc._device = device;
		desc._sizeInByte = MATERIAL_INSTANCE_COUNT_MAX * sizeof(MaterialParameter);
		desc._initialState = RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		_parameterBuffer.initialize(desc);
		_parameterBuffer.setDebugName("Material Parameters");
	}

	// descriptors
	{
		DescriptorHeapAllocator* allocater = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
		_materialParameterSrv = allocater->allocateDescriptors(1);

		ShaderResourceViewDesc desc = {};
		desc._format = FORMAT_UNKNOWN;
		desc._viewDimension = SRV_DIMENSION_BUFFER;
		desc._buffer._firstElement = 0;
		desc._buffer._flags = BUFFER_SRV_FLAG_NONE;
		desc._buffer._numElements = MATERIAL_INSTANCE_COUNT_MAX;
		desc._buffer._structureByteStride = sizeof(MaterialParameter);
		device->createShaderResourceView(_parameterBuffer.getResource(), &desc, _materialParameterSrv._cpuHandle);
	}

	_materialInstances.initialize(MATERIAL_INSTANCE_COUNT_MAX);
	_materials.initialize(MATERIAL_INSTANCE_COUNT_MAX);
}

void VramShaderSet::terminate() {
	DescriptorHeapAllocator* allocater = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	allocater->discardDescriptor(_materialParameterSrv);
	_parameterBuffer.terminate();

	LTN_ASSERT(_materialInstances.getInstanceCount() == 0);
	_materialInstances.terminate();
	_materials.terminate();
}

void VramShaderSet::updateMaterialParameter(u32 materialInstanceIndex) {
	u32 offset = sizeof(MaterialParameter) * materialInstanceIndex;
	VramBufferUpdater* vramUpdater = GraphicsSystemImpl::Get()->getVramUpdater();
	MaterialImpl* material = _materials[materialInstanceIndex];
	MaterialParameter* mapParam = vramUpdater->enqueueUpdate<MaterialParameter>(&_parameterBuffer, offset);
	mapParam->_baseColor = *material->getParameter<Color4>(StrHash32("BaseColor"));
	mapParam->_albedoTextureIndex = *material->getParameter<u32>(StrHash32("AlbedoTextureIndex"));
}

void VramShaderSet::removeMaterialInstance(u32 materialInstanceIndex) {
	_materialInstances.discard(materialInstanceIndex);
	_materials[materialInstanceIndex] = nullptr;
}

u32 VramShaderSet::addMaterialInstance(MaterialImpl* material) {
	u32 materialInstanceIndex = _materialInstances.request();
	_materials[materialInstanceIndex] = material;
	return materialInstanceIndex;
}

u32 VramShaderSet::findMaterialInstance(Material* material) const {
	u32 materialInstanceCount = _materialInstances.getResarveCount();
	u32 findIndex = gpu::INVALID_INDEX;
	for (u32 materialInstanceIndex = 0; materialInstanceIndex < materialInstanceCount; ++materialInstanceIndex) {
		if (_materials[materialInstanceIndex] == material) {
			findIndex = materialInstanceIndex;
			break;
		}
	}
	return findIndex;
}
