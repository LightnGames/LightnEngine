#include <MeshRenderer/impl/SceneImpl.h>
#include <GfxCore/impl/GraphicsSystemImpl.h>
#include <DebugRenderer/DebugRendererSystem.h>
#include <MaterialSystem/MaterialSystem.h>
#include <MaterialSystem/impl/PipelineStateSystem.h>
#include <GfxCore/impl/ViewSystemImpl.h>
#include <GfxCore/impl/QueryHeapSystem.h>
#include <Core/Application.h>
#include <MaterialSystem/impl/MaterialSystemImpl.h>

void Scene::initialize() {
	Device* device = GraphicsSystemImpl::Get()->getDevice();
	_gpuMeshInstances.initialize(MESH_INSTANCE_COUNT_MAX);
	_gpuLodMeshInstances.initialize(LOD_MESH_INSTANCE_COUNT_MAX);
	_gpuSubMeshInstances.initialize(SUB_MESH_INSTANCE_COUNT_MAX);

	// buffers
	{
		GpuBufferDesc desc = {};
		desc._sizeInByte = MESH_INSTANCE_COUNT_MAX * sizeof(gpu::MeshInstance);
		desc._initialState = RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		desc._device = device;
		_meshInstanceBuffer.initialize(desc);
		_meshInstanceBuffer.setDebugName("Mesh Instance");

		desc._sizeInByte = MESH_INSTANCE_COUNT_MAX * sizeof(Matrix43);
		_meshInstanceWorldMatrixBuffer.initialize(desc);
		_meshInstanceWorldMatrixBuffer.setDebugName("Mesh Instance World Matrix");

		desc._sizeInByte = LOD_MESH_INSTANCE_COUNT_MAX * sizeof(gpu::LodMeshInstance);
		_lodMeshInstanceBuffer.initialize(desc);
		_lodMeshInstanceBuffer.setDebugName("Lod Mesh Instance");

		desc._sizeInByte = SUB_MESH_INSTANCE_COUNT_MAX * sizeof(gpu::SubMeshInstance);
		_subMeshInstanceBuffer.initialize(desc);
		_subMeshInstanceBuffer.setDebugName("Sub Mesh Instance");

		desc._sizeInByte = GetConstantBufferAligned(sizeof(SceneCullingInfo));
		desc._initialState = RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		_sceneCullingConstantBuffer.initialize(desc);
		_sceneCullingConstantBuffer.setDebugName("Culling Constant");
	}

	// descriptors
	{
		DescriptorHeapAllocator* allocator = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
		_meshInstanceSrv = allocator->allocateDescriptors(3);
		_meshInstanceWorldMatrixSrv = allocator->allocateDescriptors(1);

		u64 incrimentSize = static_cast<u64>(allocator->getIncrimentSize());

		ShaderResourceViewDesc desc = {};
		desc._format = FORMAT_UNKNOWN;
		desc._viewDimension = SRV_DIMENSION_BUFFER;
		desc._buffer._firstElement = 0;
		desc._buffer._flags = BUFFER_SRV_FLAG_NONE;

		// mesh instance
		{
			CpuDescriptorHandle handle = _meshInstanceSrv._cpuHandle;

			desc._buffer._numElements = MESH_INSTANCE_COUNT_MAX;
			desc._buffer._structureByteStride = sizeof(gpu::MeshInstance);
			device->createShaderResourceView(_meshInstanceBuffer.getResource(), &desc, handle + incrimentSize * 0);

			desc._buffer._numElements = LOD_MESH_INSTANCE_COUNT_MAX;
			desc._buffer._structureByteStride = sizeof(gpu::LodMeshInstance);
			device->createShaderResourceView(_lodMeshInstanceBuffer.getResource(), &desc, handle + incrimentSize * 1);

			desc._buffer._numElements = SUB_MESH_INSTANCE_COUNT_MAX;
			desc._buffer._structureByteStride = sizeof(gpu::SubMeshInstance);
			device->createShaderResourceView(_subMeshInstanceBuffer.getResource(), &desc, handle + incrimentSize * 2);
		}

		{
			desc._buffer._numElements = MESH_INSTANCE_COUNT_MAX;
			desc._buffer._structureByteStride = sizeof(Matrix43);
			device->createShaderResourceView(_meshInstanceWorldMatrixBuffer.getResource(), &desc, _meshInstanceWorldMatrixSrv._cpuHandle);
		}

		// scene constant
		{
			_cullingSceneConstantHandle = allocator->allocateDescriptors(1);
			device->createConstantBufferView(_sceneCullingConstantBuffer.getConstantBufferViewDesc(), _cullingSceneConstantHandle._cpuHandle);
		}
	}


	// default shader set
	{
		ShaderSetDesc desc = {};
		desc._filePath = "common/default_mesh.sseto";
		_defaultShaderSet = MaterialSystem::Get()->createShaderSet(desc);
	}

	// default material
	{
		MaterialDesc desc = {};
		desc._filePath = "common/default_mesh.mto";
		_defaultMaterial = MaterialSystem::Get()->createMaterial(desc);
	}
}

void Scene::update() {
	VramBufferUpdater* vramUpdater = GraphicsSystemImpl::Get()->getVramUpdater();
	MaterialSystemImpl* materialSystem = MaterialSystemImpl::Get();
	bool isUpdatedVisible = false;
	u32 meshInstanceCount = _gpuMeshInstances.getResarveCount();
	for (u32 meshInstanceIndex = 0; meshInstanceIndex < meshInstanceCount; ++meshInstanceIndex) {
		if ((_meshInstanceUpdateFlags[meshInstanceIndex] & MESH_INSTANCE_UPDATE_WORLD_MATRIX)||
			(_meshInstanceUpdateFlags[meshInstanceIndex] & MESH_INSTANCE_UPDATE_VISIBLE)) {
			uploadMeshInstance(meshInstanceIndex);
			isUpdatedVisible = true;
		}
	}

	_isUpdatedInstancingOffset = false;
	u32 subMeshInstanceCount = _gpuSubMeshInstances.getResarveCount();
	for (u32 subMeshInstanceIndex = 0; subMeshInstanceIndex < subMeshInstanceCount; ++subMeshInstanceIndex) {
		if (_subMeshInstanceUpdateFlags[subMeshInstanceIndex] & SUB_MESH_INSTANCE_UPDATE_MATERIAL) {
			SubMeshInstance& subMeshInstance = _subMeshInstances[subMeshInstanceIndex];
			Material* material = subMeshInstance.getMaterial();
			subMeshInstance.setPrevMaterial(material);

			gpu::SubMeshInstance& gpuSubMeshInstance = _gpuSubMeshInstances[subMeshInstanceIndex];
			gpuSubMeshInstance._materialIndex = materialSystem->getMaterialIndex(material);
			gpuSubMeshInstance._shaderSetIndex = materialSystem->getShaderSetIndex(static_cast<MaterialImpl*>(material)->getShaderSet());

			u32 offset = sizeof(gpu::SubMeshInstance) * subMeshInstanceIndex;
			gpu::SubMeshInstance* mapSubMeshInstance = vramUpdater->enqueueUpdate<gpu::SubMeshInstance>(&_subMeshInstanceBuffer, offset);
			*mapSubMeshInstance = gpuSubMeshInstance;

			_isUpdatedInstancingOffset = true;
		}
	}

	if (isUpdatedVisible) {
		_visibleSceneInfo = SceneInfo();
		for (u32 meshInstanceIndex = 0; meshInstanceIndex < meshInstanceCount; ++meshInstanceIndex) {
			const MeshInstanceImpl& meshInstance = _meshInstances[meshInstanceIndex];
			if (!meshInstance.isVisible()) {
				continue;
			}
			const MeshInfo* meshInfo = meshInstance.getMesh()->getMeshInfo();
			++_visibleSceneInfo._meshInstanceCount;
			_visibleSceneInfo._lodMeshInstanceCount += meshInfo->_totalLodMeshCount;
			_visibleSceneInfo._subMeshInstanceCount += meshInfo->_totalSubMeshCount;
			_visibleSceneInfo._meshletInstanceCount += meshInfo->_totalMeshletCount;
			_visibleSceneInfo._vertexCount += meshInfo->_vertexCount;
			_visibleSceneInfo._triangleCount += meshInfo->_primitiveCount;
		}
	}

	SceneCullingInfo* cullingInfo = vramUpdater->enqueueUpdate<SceneCullingInfo>(&_sceneCullingConstantBuffer, 0, 1);
	cullingInfo->_meshInstanceCountMax = getMeshInstanceCountMax();
}

void Scene::processDeletion() {
	u32 meshInstanceCount = _gpuMeshInstances.getInstanceCount();
	for (u32 meshInstanceIndex = 0; meshInstanceIndex < meshInstanceCount; ++meshInstanceIndex) {
		if (_meshInstanceStateFlags[meshInstanceIndex] & MESH_INSTANCE_FLAG_REQUEST_DESTROY) {
			deleteMeshInstance(meshInstanceIndex);
			_meshInstanceStateFlags[meshInstanceIndex] = MESH_INSTANCE_FLAG_NONE;
		}
	}

	for (u32 meshInstanceIndex = 0; meshInstanceIndex < meshInstanceCount; ++meshInstanceIndex) {
		if (_meshInstanceUpdateFlags[meshInstanceIndex] & MESH_INSTANCE_UPDATE_WORLD_MATRIX) {
			_meshInstanceUpdateFlags[meshInstanceIndex] &= ~MESH_INSTANCE_UPDATE_WORLD_MATRIX;
		}

		if (_meshInstanceUpdateFlags[meshInstanceIndex] & MESH_INSTANCE_UPDATE_VISIBLE) {
			_meshInstanceUpdateFlags[meshInstanceIndex] &= ~MESH_INSTANCE_UPDATE_VISIBLE;
		}
	}

	u32 subMeshInstanceCount = _gpuSubMeshInstances.getResarveCount();
	for (u32 subMeshInstanceIndex = 0; subMeshInstanceIndex < subMeshInstanceCount; ++subMeshInstanceIndex) {
		if (_subMeshInstanceUpdateFlags[subMeshInstanceIndex] & SUB_MESH_INSTANCE_UPDATE_MATERIAL) {
			_subMeshInstanceUpdateFlags[subMeshInstanceIndex] &= ~SUB_MESH_INSTANCE_UPDATE_MATERIAL;
		}
	}
}

void Scene::terminate() {
	_meshInstanceBuffer.terminate();
	_meshInstanceWorldMatrixBuffer.terminate();
	_lodMeshInstanceBuffer.terminate();
	_subMeshInstanceBuffer.terminate();
	_sceneCullingConstantBuffer.terminate();

	LTN_ASSERT(_gpuMeshInstances.getInstanceCount() == 0);
	LTN_ASSERT(_gpuLodMeshInstances.getInstanceCount() == 0);
	LTN_ASSERT(_gpuSubMeshInstances.getInstanceCount() == 0);

	_gpuMeshInstances.terminate();
	_gpuLodMeshInstances.terminate();
	_gpuSubMeshInstances.terminate();

	DescriptorHeapAllocator* allocator = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	allocator->discardDescriptor(_meshInstanceSrv);
	allocator->discardDescriptor(_meshInstanceWorldMatrixSrv);
	allocator->discardDescriptor(_cullingSceneConstantHandle);
}

void Scene::terminateDefaultResources() {
	_defaultShaderSet->requestToDelete();
	_defaultMaterial->requestToDelete();
}

void Scene::uploadMeshInstance(u32 meshInstanceIndex) {
	MeshInstanceImpl& meshInstance = _meshInstances[meshInstanceIndex];
	gpu::MeshInstance& gpuMeshInstance = _gpuMeshInstances[meshInstanceIndex];
	const MeshInfo* meshInfo = meshInstance.getMesh()->getMeshInfo();
	Matrix4 matrixWorld = meshInstance.getWorldMatrix();
	Matrix4 transposedMatrixWorld = matrixWorld.transpose();
	Vector3 worldScale = matrixWorld.getScale();

	u32 stateFlags = 0;
	if (meshInstance.isEnabled()) {
		stateFlags |= gpu::MESH_INSTANCE_STATE_FLAGS_ENABLE;
	}

	if(meshInstance.isVisible()) {
		stateFlags |= gpu::MESH_INSTANCE_STATE_FLAGS_VISIBLE;
	}

	AABB meshInstanceLocalBounds(meshInfo->_boundsMin, meshInfo->_boundsMax);
	AABB meshInstanceBounds = meshInstanceLocalBounds.getTransformedAabb(matrixWorld);
	Vector3 boundsCenter = (meshInstanceBounds._min + meshInstanceBounds._max) / 2.0f;
	f32 boundsRadius = Vector3::length(meshInstanceBounds._max - boundsCenter);
	gpuMeshInstance._aabbMin = meshInstanceBounds._min.getFloat3();
	gpuMeshInstance._aabbMax = meshInstanceBounds._max.getFloat3();
	gpuMeshInstance._stateFlags = stateFlags;
	gpuMeshInstance._boundsRadius = boundsRadius;
	gpuMeshInstance._worldScale = Max(worldScale._x, Max(worldScale._y, worldScale._z));

	VramBufferUpdater* vramUpdater = GraphicsSystemImpl::Get()->getVramUpdater();
	gpu::MeshInstance* mapMeshInstance = vramUpdater->enqueueUpdate<gpu::MeshInstance>(&_meshInstanceBuffer, sizeof(gpu::MeshInstance) * meshInstanceIndex);
	*mapMeshInstance = gpuMeshInstance;

	Matrix43* mapMeshInstanceWorldMatrix = vramUpdater->enqueueUpdate<Matrix43>(&_meshInstanceWorldMatrixBuffer, sizeof(Matrix43) * meshInstanceIndex);
	*mapMeshInstanceWorldMatrix = transposedMatrixWorld.getMatrix43();
}

void Scene::deleteMeshInstance(u32 meshInstanceIndex) {
	const MeshInstanceImpl& meshInstanceInfo = _meshInstances[meshInstanceIndex];
	const MeshInfo* meshInfo = meshInstanceInfo.getMesh()->getMeshInfo();

	u32 lodMeshCount = meshInfo->_totalLodMeshCount;
	u32 subMeshCount = meshInfo->_totalSubMeshCount;
	const gpu::MeshInstance& meshInstance = _gpuMeshInstances[meshInstanceIndex];
	const gpu::LodMeshInstance& lodMeshInstance = _gpuLodMeshInstances[meshInstance._lodMeshInstanceOffset];
	const gpu::SubMeshInstance& subMeshInstance = _gpuSubMeshInstances[lodMeshInstance._subMeshInstanceOffset];
	_gpuSubMeshInstances.discard(&_gpuSubMeshInstances[lodMeshInstance._subMeshInstanceOffset], subMeshCount);
	_gpuLodMeshInstances.discard(&_gpuLodMeshInstances[meshInstance._lodMeshInstanceOffset], lodMeshCount);
	_gpuMeshInstances.discard(&_gpuMeshInstances[meshInstanceIndex], 1);

	_meshInstances[meshInstanceIndex] = MeshInstanceImpl();

	VramBufferUpdater* vramUpdater = GraphicsSystemImpl::Get()->getVramUpdater();
	gpu::MeshInstance* mapMeshInstance = vramUpdater->enqueueUpdate<gpu::MeshInstance>(&_meshInstanceBuffer, sizeof(gpu::MeshInstance) * meshInstanceIndex);
	*mapMeshInstance = gpu::MeshInstance();

	_sceneInfo._meshInstanceCount--;
	_sceneInfo._lodMeshInstanceCount -= meshInfo->_totalLodMeshCount;
	_sceneInfo._subMeshInstanceCount -= meshInfo->_totalSubMeshCount;
	_sceneInfo._meshletInstanceCount -= meshInfo->_totalMeshletCount;
	_sceneInfo._vertexCount -= meshInfo->_vertexCount;
	_sceneInfo._triangleCount -= meshInfo->_primitiveCount;
}

void Scene::debugDrawMeshInstanceBounds() {
	for (u32 meshInstanceIndex = 0; meshInstanceIndex < MESH_INSTANCE_COUNT_MAX; ++meshInstanceIndex) {
		if (_meshInstanceStateFlags[meshInstanceIndex] == MESH_INSTANCE_FLAG_NONE) {
			continue;
		}

		const gpu::MeshInstance& meshInstance = _gpuMeshInstances[meshInstanceIndex];
		Vector3 boundsMin = Vector3(meshInstance._aabbMin);
		Vector3 boundsMax = Vector3(meshInstance._aabbMax);
		DebugRendererSystem::Get()->drawAabb(boundsMin, boundsMax, Color4::BLUE);
	}
}

void Scene::debugDrawGui() {
	u32 meshInstanceCount = _gpuMeshInstances.getResarveCount();
	DebugGui::Text("Total:%3d", meshInstanceCount);
	DebugGui::Columns(2, "tree", true);
	constexpr f32 MESH_NAME_WIDTH = 320;
	DebugGui::SetColumnWidth(0, MESH_NAME_WIDTH);
	for (u32 x = 0; x < meshInstanceCount; x++) {
		if (_meshInstanceStateFlags[x] == MESH_INSTANCE_FLAG_NONE) {
			DebugGui::TextDisabled("Disabled");
			DebugGui::NextColumn();
			DebugGui::TextDisabled("Disabled");
			DebugGui::NextColumn();
			continue;
		}
		MeshInstanceImpl& meshInstance = _meshInstances[x];
		gpu::MeshInstance& gpuMeshInstance = _gpuMeshInstances[x];
		const MeshInfo* meshInfo = meshInstance.getMesh()->getMeshInfo();
		const SubMeshInfo* subMeshInfos = meshInstance.getMesh()->getSubMeshInfo();
		const DebugMeshInfo* debugMeshInfo = meshInstance.getMesh()->getDebugMeshInfo();
		bool open1 = DebugGui::TreeNode(static_cast<s32>(x), "%-3d: %-20s", x, debugMeshInfo->_filePath);
		Color4 lodCountTextColor = open1 ? Color4::GREEN : Color4::WHITE;
		DebugGui::NextColumn();
		DebugGui::TextColored(lodCountTextColor, "Lod Count:%-2d", meshInfo->_totalLodMeshCount);
		DebugGui::NextColumn();
		if (!open1) {
			continue;
		}

		// mesh info
		DebugGui::Separator();
		u32 subMeshIndex = 0;
		for (u32 y = 0; y < meshInfo->_totalLodMeshCount; y++) {
			bool open2 = DebugGui::TreeNode(static_cast<s32>(y), "Lod %2d", y);
			u32 subMeshCount = meshInstance.getMesh()->getGpuLodMesh(y)->_subMeshCount;
			Color4 subMeshCountTextColor = open2 ? Color4::GREEN : Color4::WHITE;
			DebugGui::NextColumn();
			DebugGui::TextColored(subMeshCountTextColor, "Sub Mesh Count:%2d", subMeshCount);
			DebugGui::NextColumn();
			if (!open2) {
				continue;
			}

			DebugGui::Separator();
			for (u32 z = 0; z < subMeshCount; z++) {
				bool open3 = DebugGui::TreeNode(static_cast<s32>(z), "Sub Mesh %2d", z);
				Color4 meshletCountTextColor = open3 ? Color4::GREEN : Color4::WHITE;
				u32 meshletCount = subMeshInfos[z]._meshletCount;
				DebugGui::NextColumn();
				DebugGui::TextColored(meshletCountTextColor, "Meshlet Count:%2d", meshletCount);
				DebugGui::NextColumn();

				if (!open3) {
					continue;
				}

				DebugGui::Separator();
				for (u32 w = 0; w < meshletCount; w++) {
					DebugGui::Text("Meshlet %2d", w);
				}
				// メッシュレット情報
				DebugGui::TreePop();
			}
			subMeshIndex += subMeshCount;
			DebugGui::TreePop();
		}

		constexpr char boundsFormat[] = "%-14s:[%5.1f][%5.1f][%5.1f]";
		Matrix4 matrixWorld = meshInstance.getWorldMatrix();
		DebugGui::Text(boundsFormat, "World Matrix 0", matrixWorld.m[0][0], matrixWorld.m[0][1], matrixWorld.m[0][2]);
		DebugGui::Text(boundsFormat, "World Matrix 1", matrixWorld.m[1][0], matrixWorld.m[1][1], matrixWorld.m[1][2]);
		DebugGui::Text(boundsFormat, "World Matrix 2", matrixWorld.m[2][0], matrixWorld.m[2][1], matrixWorld.m[2][2]);
		DebugGui::Text(boundsFormat, "World Matrix 3", matrixWorld.m[3][0], matrixWorld.m[3][1], matrixWorld.m[3][2]);

		DebugGui::Text(boundsFormat, "Bounds Min", gpuMeshInstance._aabbMin._x, gpuMeshInstance._aabbMin._y, gpuMeshInstance._aabbMin._z);
		DebugGui::Text(boundsFormat, "Bounds Max", gpuMeshInstance._aabbMax._x, gpuMeshInstance._aabbMax._y, gpuMeshInstance._aabbMax._z);

		DebugGui::TreePop();
	}
	DebugGui::Columns(1);

	DebugGui::EndTabItem();
}

void Scene::createMeshInstances(MeshInstance** outMeshInstances, const Mesh** meshes, u32 instanceCount) {
	u32 totalLodMeshCount = 0;
	u32 totalSubMeshCount = 0;
	for (u32 meshInstanceIndex = 0; meshInstanceIndex < instanceCount; ++meshInstanceIndex) {
		const MeshInfo* meshInfo = meshes[meshInstanceIndex]->getMeshInfo();
		totalLodMeshCount += meshInfo->_totalLodMeshCount;
		totalSubMeshCount += meshInfo->_totalSubMeshCount;
	}

	// メッシュインスタンス確保
	u32 meshInstanceIndex = _gpuMeshInstances.request(instanceCount);
	u32 lodMeshInstanceIndex = _gpuLodMeshInstances.request(totalLodMeshCount);
	u32 subMeshInstanceIndex = _gpuSubMeshInstances.request(totalSubMeshCount);

	// メッシュインスタンスに LOD・サブメッシュインスタンス・各種フラグをセット
	u32 totalLodMeshCounter = 0;
	u32 totalSubMeshCounter = 0;
	for (u32 instanceIndex = 0; instanceIndex < instanceCount; ++instanceIndex) {
		const MeshInfo* meshInfo = meshes[instanceIndex]->getMeshInfo();
		u32 globalMeshInstanceIndex = meshInstanceIndex + instanceIndex;
		u32 globalLodMeshInstanceIndex = lodMeshInstanceIndex + totalLodMeshCounter;
		u32 globalSubMeshInstanceIndex = subMeshInstanceIndex + totalSubMeshCounter;
		MeshInstanceImpl* meshInstance = &_meshInstances[globalMeshInstanceIndex];
		meshInstance->setMesh(meshes[instanceIndex]);
		meshInstance->setGpuMeshInstance(&_gpuMeshInstances[globalMeshInstanceIndex]);
		meshInstance->setGpuLodMeshInstances(&_gpuLodMeshInstances[globalLodMeshInstanceIndex]);
		meshInstance->setGpuSubMeshInstances(&_gpuSubMeshInstances[globalSubMeshInstanceIndex]);
		meshInstance->setSubMeshInstance(&_subMeshInstances[globalSubMeshInstanceIndex]);
		meshInstance->setUpdateFlags(&_meshInstanceUpdateFlags[globalMeshInstanceIndex]);
		meshInstance->setStateFlags(&_meshInstanceStateFlags[globalMeshInstanceIndex]);
		meshInstance->setWorldMatrix(Matrix4::Identity);
		meshInstance->setVisiblity(true);
		meshInstance->setEnabled();

		totalLodMeshCounter += meshInfo->_totalLodMeshCount;
		totalSubMeshCounter += meshInfo->_totalSubMeshCount;
		outMeshInstances[instanceIndex] = meshInstance;
	}

	for (u32 instanceIndex = 0; instanceIndex < instanceCount; ++instanceIndex) {
		const MeshInfo* meshInfo = meshes[instanceIndex]->getMeshInfo();
		_sceneInfo._meshInstanceCount++;
		_sceneInfo._lodMeshInstanceCount += meshInfo->_totalLodMeshCount;
		_sceneInfo._subMeshInstanceCount += meshInfo->_totalSubMeshCount;
		_sceneInfo._meshletInstanceCount += meshInfo->_totalMeshletCount;
		_sceneInfo._vertexCount += meshInfo->_vertexCount;
		_sceneInfo._triangleCount += meshInfo->_primitiveCount;
	}

	// VRAM 用メッシュインスタンスに値を詰める
	MaterialSystemImpl* materialSystem = MaterialSystemImpl::Get();
	u32 defaultMaterialIndex = materialSystem->getMaterialIndex(_defaultMaterial);
	u32 defaultShaderSetIndex = materialSystem->getShaderSetIndex(static_cast<MaterialImpl*>(_defaultMaterial)->getShaderSet());
	for (u32 instanceIndex = 0; instanceIndex < instanceCount; ++instanceIndex) {
		MeshInstanceImpl* meshInstance = &_meshInstances[meshInstanceIndex + instanceIndex];
		const Mesh* mesh = meshInstance->getMesh();
		const MeshInfo* meshInfo = mesh->getMeshInfo();
		u32 lodMeshCount = meshInfo->_totalLodMeshCount;
		u32 subMeshCount = meshInfo->_totalSubMeshCount;
		u32 lodMeshInstanceOffset = static_cast<u32>(meshInstance->getGpuLodMeshInstance(0) - &_gpuLodMeshInstances[0]);
		u32 subMeshInstanceOffset = static_cast<u32>(meshInstance->getGpuSubMeshInstance(0) - &_gpuSubMeshInstances[0]);

		gpu::MeshInstance* gpuMeshInstance = meshInstance->getGpuMeshInstance();
		gpuMeshInstance->_lodMeshInstanceOffset = lodMeshInstanceOffset;
		gpuMeshInstance->_meshIndex = meshInfo->_meshIndex;
		for (u32 lodMeshIndex = 0; lodMeshIndex < lodMeshCount; ++lodMeshIndex) {
			f32 threshhold = 1.0f - ((lodMeshIndex + 1) / static_cast<f32>(lodMeshCount));
			gpu::LodMeshInstance* lodMeshInstance = meshInstance->getGpuLodMeshInstance(lodMeshIndex);
			lodMeshInstance->_subMeshInstanceOffset = meshInfo->_subMeshOffsets[lodMeshIndex] + subMeshInstanceOffset;
			lodMeshInstance->_threshhold = threshhold;
		}

		for (u32 subMeshIndex = 0; subMeshIndex < subMeshCount; ++subMeshIndex) {
			gpu::SubMeshInstance* subMeshInstance = meshInstance->getGpuSubMeshInstance(subMeshIndex);
			subMeshInstance->_materialIndex = defaultMaterialIndex;
			subMeshInstance->_shaderSetIndex = defaultShaderSetIndex;
		}
	}

	// CPU 用サブメッシュインスタンスデータセット
	for (u32 subMeshIndex = 0; subMeshIndex < totalSubMeshCount; ++subMeshIndex) {
		u32 index = subMeshInstanceIndex + subMeshIndex;
		SubMeshInstanceImpl& subMeshInstance = _subMeshInstances[index];
		subMeshInstance.setUpdateFlags(&_subMeshInstanceUpdateFlags[index]);
		subMeshInstance.setMaterial(_defaultMaterial);
		subMeshInstance.setPrevMaterial(_defaultMaterial);
	}

	// VRAM にアップロード
	VramBufferUpdater* vramUpdater = GraphicsSystemImpl::Get()->getVramUpdater();
	u32 lodMeshInstanceOffsetByte = sizeof(gpu::LodMeshInstance) * lodMeshInstanceIndex;
	gpu::LodMeshInstance* lodMeshInstance = vramUpdater->enqueueUpdate<gpu::LodMeshInstance>(&_lodMeshInstanceBuffer, lodMeshInstanceOffsetByte, totalLodMeshCount);
	memcpy(lodMeshInstance, &_gpuLodMeshInstances[lodMeshInstanceIndex], sizeof(gpu::LodMeshInstance) * totalLodMeshCount);

	u32 subMeshInstanceOffsetByte = sizeof(gpu::SubMeshInstance) * subMeshInstanceIndex;
	gpu::SubMeshInstance* subMeshInstance = vramUpdater->enqueueUpdate<gpu::SubMeshInstance>(&_subMeshInstanceBuffer, subMeshInstanceOffsetByte, totalSubMeshCount);
	memcpy(subMeshInstance, &_gpuSubMeshInstances[subMeshInstanceIndex], sizeof(gpu::SubMeshInstance) * totalSubMeshCount);

	u32 meshInstanceOffsetByte = sizeof(gpu::MeshInstance) * meshInstanceIndex;
	gpu::MeshInstance* mapMeshInstance = vramUpdater->enqueueUpdate<gpu::MeshInstance>(&_meshInstanceBuffer, meshInstanceOffsetByte, instanceCount);
	memcpy(mapMeshInstance, &_gpuMeshInstances[meshInstanceIndex], sizeof(gpu::MeshInstance) * instanceCount);
}

void MeshInstance::requestToDelete() {
	*_stateFlags |= MESH_INSTANCE_FLAG_REQUEST_DESTROY;
}

void MeshInstance::setWorldMatrix(const Matrix4& matrixWorld) {
	_matrixWorld = matrixWorld;
	*_updateFlags |= MESH_INSTANCE_UPDATE_WORLD_MATRIX;
}

void MeshInstance::setMaterialSlotIndex(Material* material, u32 slotIndex) {
	const gpu::SubMesh* subMeshes = _mesh->getGpuSubMesh();
	u32 subMeshCount = _mesh->getMeshInfo()->_totalSubMeshCount;
	for (u32 subMeshIndex = 0; subMeshIndex < subMeshCount; ++subMeshIndex) {
		if (subMeshes[subMeshIndex]._materialSlotIndex == slotIndex) {
			_subMeshInstances[subMeshIndex].setMaterial(material);
		}
	}
}

void MeshInstance::setMaterial(Material* material, u64 slotNameHash) {
	u32 slotIndex = getMaterialSlotIndex(slotNameHash);
	LTN_ASSERT(slotIndex != gpu::INVALID_INDEX);
	setMaterialSlotIndex(material, slotIndex);
}

void MeshInstance::setVisiblity(bool visible) {
	_visiblity = visible;
	*_updateFlags |= MESH_INSTANCE_UPDATE_VISIBLE;
}

u32 MeshInstance::getMaterialSlotIndex(u64 slotNameHash) const {
	const u32 materialSlotCount = getMesh()->getMeshInfo()->_materialSlotCount;
	const u64* materialSlotNameHashes = getMesh()->getMeshInfo()->_materialSlotNameHashes;
	u32 slotIndex = static_cast<u32>(-1);
	for (u32 materialIndex = 0; materialIndex < materialSlotCount; ++materialIndex) {
		if (materialSlotNameHashes[materialIndex] == slotNameHash) {
			slotIndex = materialIndex;
			break;
		}
	}
	return slotIndex;
}

void SubMeshInstanceImpl::setMaterial(Material* material) {
	if (_material == material) {
		return;
	}

	_material = material;
	*_updateFlags |= SUB_MESH_INSTANCE_UPDATE_MATERIAL;
}

void SubMeshInstanceImpl::setDefaultMaterial(Material* material) {
	_material = material;
}

void IndirectArgumentResource::initialize(const InitializeDesc& initializeDesc) {
	Device* device = GraphicsSystemImpl::Get()->getDevice();
	DescriptorHeapAllocator* allocator = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	DescriptorHeapAllocator* cpuAllocator = GraphicsSystemImpl::Get()->getSrvCbvUavCpuDescriptorAllocator();

	// indirect argument buffers
	{
		GpuBufferDesc desc = {};
		desc._sizeInByte = initializeDesc._strideInByte * initializeDesc._indirectArgumentCount;
		desc._initialState = RESOURCE_STATE_INDIRECT_ARGUMENT;
		desc._flags = RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		desc._device = device;
		_indirectArgumentBuffer.initialize(desc);
		_indirectArgumentBuffer.setDebugName("Indirect Argument");

		desc._sizeInByte = sizeof(u32) * initializeDesc._indirectArgumentCounterCount;
		_countBuffer.initialize(desc);
		_countBuffer.setDebugName("Indirect Argument Count");
	}

	// indirect argument uav
	{
		_indirectArgumentUavHandle = allocator->allocateDescriptors(2);
		u32 incrimentSize = allocator->getIncrimentSize();
		CpuDescriptorHandle indirectArgumentHandle = _indirectArgumentUavHandle._cpuHandle;
		CpuDescriptorHandle countHandle = indirectArgumentHandle + incrimentSize;

		UnorderedAccessViewDesc desc = {};
		desc._format = FORMAT_UNKNOWN;
		desc._viewDimension = UAV_DIMENSION_BUFFER;
		desc._buffer._firstElement = 0;
		desc._buffer._numElements = initializeDesc._indirectArgumentCount;
		desc._buffer._structureByteStride = initializeDesc._strideInByte;
		device->createUnorderedAccessView(_indirectArgumentBuffer.getResource(), nullptr, &desc, indirectArgumentHandle);

		desc._buffer._numElements = initializeDesc._indirectArgumentCounterCount;
		desc._buffer._flags = BUFFER_UAV_FLAG_RAW;
		desc._buffer._structureByteStride = 0;
		desc._format = FORMAT_R32_TYPELESS;
		device->createUnorderedAccessView(_countBuffer.getResource(), nullptr, &desc, countHandle);

		// カウントバッファを API の機能でクリアするために CPU Only Descriptor を作成
		_countCpuUav = cpuAllocator->allocateDescriptors(1);
		device->createUnorderedAccessView(_countBuffer.getResource(), nullptr, &desc, _countCpuUav._cpuHandle);
	}
	_indirectArgumentCountMax = initializeDesc._indirectArgumentCount;
	_indirectArgumentCounterCountMax = initializeDesc._indirectArgumentCounterCount;
}

void IndirectArgumentResource::terminate() {
	_indirectArgumentBuffer.terminate();
	_countBuffer.terminate();

	DescriptorHeapAllocator* allocator = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	allocator->discardDescriptor(_indirectArgumentUavHandle);

	DescriptorHeapAllocator* cpuAllocator = GraphicsSystemImpl::Get()->getSrvCbvUavCpuDescriptorAllocator();
	cpuAllocator->discardDescriptor(_countCpuUav);
}

void GpuCullingResource::setComputeLodResource(CommandList* commandList) {
	commandList->setComputeRootDescriptorTable(GpuComputeLodRootParam::RESULT_LEVEL, _currentLodLevelUav._gpuHandle);
}

void GpuCullingResource::setGpuCullingResources(CommandList* commandList) {
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::LOD_LEVEL, _currentLodLevelSrv._gpuHandle);
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::CULLING_RESULT, _gpuCullingResultUavHandle._gpuHandle);
	commandList->setComputeRootDescriptorTable(GpuCullingRootParam::HIZ, _hizDepthTextureSrv._gpuHandle);
}

void GpuCullingResource::setHizResourcesPass0(CommandList* commandList) {
	commandList->setComputeRootDescriptorTable(BuildHizRootParameters::HIZ_INFO, _hizInfoConstantCbv[0]._gpuHandle);
	commandList->setComputeRootDescriptorTable(BuildHizRootParameters::OUTPUT_DEPTH, _hizDepthTextureUav._gpuHandle); // 0 ~ 3
}

void GpuCullingResource::setHizResourcesPass1(CommandList* commandList) {
	DescriptorHeapAllocator* descriptorHeapAllocater = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	u64 incrimentSize = static_cast<u64>(descriptorHeapAllocater->getIncrimentSize());

	commandList->setComputeRootDescriptorTable(BuildHizRootParameters::HIZ_INFO, _hizInfoConstantCbv[1]._gpuHandle);
	commandList->setComputeRootDescriptorTable(BuildHizRootParameters::OUTPUT_DEPTH, _hizDepthTextureUav._gpuHandle + incrimentSize * 4); // 4 ~ 5
	commandList->setComputeRootDescriptorTable(BuildHizRootParameters::INPUT_DEPTH, _hizDepthTextureSrv._gpuHandle + incrimentSize * 3); // 3
}

void GpuCullingResource::resourceBarriersComputeLodToUAV(CommandList* commandList) {
	_currentLodLevelBuffer.transitionResource(commandList, RESOURCE_STATE_UNORDERED_ACCESS);
}

void GpuCullingResource::resetResourceComputeLodBarriers(CommandList* commandList) {
	_currentLodLevelBuffer.transitionResource(commandList, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void GpuCullingResource::resourceBarriersHizTextureToUav(CommandList* commandList, u32 offset) {
	ResourceTransitionBarrier srvToUav[4] = {};
	for (u32 i = 0; i < LTN_COUNTOF(srvToUav); ++i) {
		srvToUav[i] = _hizDepthTextures[i + offset].getAndUpdateTransitionBarrier(RESOURCE_STATE_UNORDERED_ACCESS);
	}
	commandList->transitionBarriers(srvToUav, LTN_COUNTOF(srvToUav));
}

void GpuCullingResource::resourceBarriersHizUavtoSrv(CommandList* commandList, u32 offset) {
	ResourceTransitionBarrier uavToSrv[4] = {};
	for (u32 i = 0; i < LTN_COUNTOF(uavToSrv); ++i) {
		uavToSrv[i] = _hizDepthTextures[i + offset].getAndUpdateTransitionBarrier(RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
	commandList->transitionBarriers(uavToSrv, LTN_COUNTOF(uavToSrv));
}

void GpuCullingResource::resourceBarriersHizSrvToTexture(CommandList* commandList) {
	ResourceTransitionBarrier srvToTexture[gpu::HIERACHICAL_DEPTH_COUNT] = {};
	for (u32 i = 0; i < gpu::HIERACHICAL_DEPTH_COUNT; ++i) {
		srvToTexture[i] = _hizDepthTextures[i].getAndUpdateTransitionBarrier(RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	commandList->transitionBarriers(srvToTexture, LTN_COUNTOF(srvToTexture));
}

void GpuCullingResource::resourceBarriersHizTextureToSrv(CommandList* commandList) {
	ResourceTransitionBarrier textureToSrv[gpu::HIERACHICAL_DEPTH_COUNT] = {};
	for (u32 i = 0; i < gpu::HIERACHICAL_DEPTH_COUNT; ++i) {
		textureToSrv[i] = _hizDepthTextures[i].getAndUpdateTransitionBarrier(RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
	commandList->transitionBarriers(textureToSrv, LTN_COUNTOF(textureToSrv));
}

void IndirectArgumentResource::resourceBarriersToUav(CommandList* commandList) {
	ResourceTransitionBarrier uavToIndirectArgumentBarriers[2] = {};
	uavToIndirectArgumentBarriers[0] = _indirectArgumentBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_UNORDERED_ACCESS);
	uavToIndirectArgumentBarriers[1] = _countBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_UNORDERED_ACCESS);
	commandList->transitionBarriers(uavToIndirectArgumentBarriers, LTN_COUNTOF(uavToIndirectArgumentBarriers));
}

void IndirectArgumentResource::resourceBarriersToIndirectArgument(CommandList* commandList) {
	ResourceTransitionBarrier uavToIndirectArgumentBarriers[2] = {};
	uavToIndirectArgumentBarriers[0] = _indirectArgumentBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_INDIRECT_ARGUMENT);
	uavToIndirectArgumentBarriers[1] = _countBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_INDIRECT_ARGUMENT);
	commandList->transitionBarriers(uavToIndirectArgumentBarriers, LTN_COUNTOF(uavToIndirectArgumentBarriers));
}

// カウントバッファクリア
void IndirectArgumentResource::resetIndirectArgumentCountBuffers(CommandList* commandList) {
	u32 clearValues[4] = {};
	DescriptorHeapAllocator* allocater = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	u32 incrimentSize = allocater->getIncrimentSize();
	GpuDescriptorHandle gpuDescriptor = _indirectArgumentUavHandle._gpuHandle + incrimentSize;
	CpuDescriptorHandle cpuDescriptor = _countCpuUav._cpuHandle;
	commandList->clearUnorderedAccessViewUint(gpuDescriptor, cpuDescriptor, _countBuffer.getResource(), clearValues, 0, nullptr);
}

void IndirectArgumentResource::executeIndirect(CommandList* commandList, CommandSignature* commandSignature, u32 commandCountMax, u32 indirectArgumentOffset, u32 countBufferOffset) {
	Resource* indirectArgumentResource = _indirectArgumentBuffer.getResource();
	Resource* countResource = _countBuffer.getResource();
	commandList->executeIndirect(commandSignature, commandCountMax, indirectArgumentResource, indirectArgumentOffset, countResource, countBufferOffset);
}

// カリング結果バッファクリア
void GpuCullingResource::resetResultBuffers(CommandList* commandList) {
	u32 clearValues[4] = {};
	{
		GpuDescriptorHandle gpuDescriptor = _gpuCullingResultUavHandle._gpuHandle;
		CpuDescriptorHandle cpuDescriptor = _gpuCullingResultCpuUavHandle._cpuHandle;
		commandList->clearUnorderedAccessViewUint(gpuDescriptor, cpuDescriptor, _gpuCullingResultBuffer.getResource(), clearValues, 0, nullptr);
	}

	{
		GpuDescriptorHandle gpuDescriptor = _amplificationCullingResultUavHandle._gpuHandle;
		CpuDescriptorHandle cpuDescriptor = _amplificationCullingResultCpuUavHandle._cpuHandle;
		commandList->clearUnorderedAccessViewUint(gpuDescriptor, cpuDescriptor, _amplificationCullingResultBuffer.getResource(), clearValues, 0, nullptr);
	}
}

void GpuCullingResource::readbackCullingResultBuffer(CommandList* commandList) {
	// カリング結果をリードバックバッファへコピー 
	u64 frameIndex = static_cast<u64>(GraphicsSystemImpl::Get()->getFrameIndex());
	MemoryRange range(frameIndex, frameIndex + 1);
	{
		u32 offset = static_cast<u32>(frameIndex) * sizeof(gpu::GpuCullingResult);
		_gpuCullingResultBuffer.transitionResource(commandList, RESOURCE_STATE_COPY_SOURCE);
		commandList->copyBufferRegion(_gpuCullingResultReadbackBuffer.getResource(), offset, _gpuCullingResultBuffer.getResource(), 0, sizeof(gpu::GpuCullingResult));
		_gpuCullingResultBuffer.transitionResource(commandList, RESOURCE_STATE_UNORDERED_ACCESS);

		gpu::GpuCullingResult* mapPtr = _gpuCullingResultReadbackBuffer.map<gpu::GpuCullingResult>(&range);
		memcpy(&_currentFrameGpuCullingResult, mapPtr, sizeof(gpu::GpuCullingResult));
		_gpuCullingResultReadbackBuffer.unmap();
	}

	{
		u32 offset = static_cast<u32>(frameIndex) * sizeof(gpu::AmplificationCullingResult);
		_amplificationCullingResultBuffer.transitionResource(commandList, RESOURCE_STATE_COPY_SOURCE);
		commandList->copyBufferRegion(_amplificationCullingResultReadbackBuffer.getResource(), offset, _amplificationCullingResultBuffer.getResource(), 0, sizeof(gpu::AmplificationCullingResult));
		_amplificationCullingResultBuffer.transitionResource(commandList, RESOURCE_STATE_UNORDERED_ACCESS);

		gpu::AmplificationCullingResult* mapPtr = _amplificationCullingResultReadbackBuffer.map<gpu::AmplificationCullingResult>(&range);
		memcpy(&_currentFrameAmplificationCullingResult, mapPtr, sizeof(gpu::AmplificationCullingResult));
		_amplificationCullingResultReadbackBuffer.unmap();
	}
}

void GpuCullingResource::setDrawResultDescriptorTable(CommandList* commandList) {
	commandList->setGraphicsRootDescriptorTable(DefaultMeshRootParam::CULLING_RESULT, _amplificationCullingResultUavHandle._gpuHandle);
	commandList->setGraphicsRootDescriptorTable(DefaultMeshRootParam::HIZ, _hizDepthTextureSrv._gpuHandle);
}

void GpuCullingResource::setDrawCurrentLodDescriptorTable(CommandList* commandList) {
	commandList->setGraphicsRootDescriptorTable(DefaultMeshRootParam::LOD_LEVEL, _currentLodLevelSrv._gpuHandle);
}

ResourceDesc GpuCullingResource::getHizTextureResourceDesc(u32 level) const {
	return _hizDepthTextures[level].getResourceDesc();
}

const gpu::GpuCullingResult* GpuCullingResource::getGpuCullingResult() const {
	return &_currentFrameGpuCullingResult;
}

const gpu::AmplificationCullingResult* GpuCullingResource::getAmplificationCullingResult() const {
	return &_currentFrameAmplificationCullingResult;
}

void InstancingResource::initialize() {
	Device* device = GraphicsSystemImpl::Get()->getDevice();
	DescriptorHeapAllocator* allocator = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	DescriptorHeapAllocator* cpuAllocator = GraphicsSystemImpl::Get()->getSrvCbvUavCpuDescriptorAllocator();

	// buffer
	{
		GpuBufferDesc desc = {};
		desc._sizeInByte = INDIRECT_ARGUMENT_COUNTER_COUNT_MAX * sizeof(u32);
		desc._initialState = RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		desc._device = device;
		_infoOffsetBuffer.initialize(desc);
		_infoOffsetBuffer.setDebugName("Primitive Instancing Offsets");

		desc._flags = RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		desc._sizeInByte = INDIRECT_ARGUMENT_COUNTER_COUNT_MAX * sizeof(u32);
		_infoCountBuffer.initialize(desc);
		_infoCountBuffer.setDebugName("Primitive Instancing Counts");

		desc._sizeInByte = INDIRECT_ARGUMENT_COUNTER_COUNT_MAX * sizeof(gpu::MeshletInstancePrimitiveInfo);
		_primitiveInfoBuffer.initialize(desc);
		_primitiveInfoBuffer.setDebugName("Meshlet Primitive Instance Info");

		desc._sizeInByte = INDIRECT_ARGUMENT_COUNTER_COUNT_MAX * sizeof(u32);
		_primitiveInfoMeshInstanceIndexBuffer.initialize(desc);
		_primitiveInfoMeshInstanceIndexBuffer.setDebugName("Meshlet Primitive Instance Mesh Instance Index");
	}

	// srv
	{
		_infoOffsetSrv = allocator->allocateDescriptors(1);
		_infoCountSrv = allocator->allocateDescriptors(1);
		_primitiveInfoSrv = allocator->allocateDescriptors(1);
		_primitiveInfoMeshInstanceIndexSrv = allocator->allocateDescriptors(1);

		ShaderResourceViewDesc desc = {};
		desc._viewDimension = SRV_DIMENSION_BUFFER;
		desc._format = FORMAT_R32_TYPELESS;
		desc._buffer._flags = BUFFER_SRV_FLAG_RAW;
		desc._buffer._numElements = INDIRECT_ARGUMENT_COUNTER_COUNT_MAX;
		device->createShaderResourceView(_infoOffsetBuffer.getResource(), &desc, _infoOffsetSrv._cpuHandle);
		device->createShaderResourceView(_infoCountBuffer.getResource(), &desc, _infoCountSrv._cpuHandle);

		desc._format = FORMAT_UNKNOWN;
		desc._buffer._flags = BUFFER_SRV_FLAG_NONE;
		desc._buffer._numElements = INDIRECT_ARGUMENT_COUNTER_COUNT_MAX;
		desc._buffer._structureByteStride = sizeof(gpu::MeshletInstancePrimitiveInfo);
		device->createShaderResourceView(_primitiveInfoBuffer.getResource(), &desc, _primitiveInfoSrv._cpuHandle);

		desc._buffer._structureByteStride = sizeof(u32);
		device->createShaderResourceView(_primitiveInfoMeshInstanceIndexBuffer.getResource(), &desc, _primitiveInfoMeshInstanceIndexSrv._cpuHandle);
	}

	// uav
	{
		u32 incrimentSize = allocator->getIncrimentSize();
		_infoCountUav = allocator->allocateDescriptors(1);
		_primitiveInfoUav = allocator->allocateDescriptors(1);
		_primitiveInfoMeshInstanceIndexUav = allocator->allocateDescriptors(1);
		_infoCountCpuUav = cpuAllocator->allocateDescriptors(1);

		UnorderedAccessViewDesc desc = {};
		desc._format = FORMAT_UNKNOWN;
		desc._viewDimension = UAV_DIMENSION_BUFFER;
		desc._buffer._firstElement = 0;
		desc._buffer._numElements = INDIRECT_ARGUMENT_COUNTER_COUNT_MAX;
		desc._buffer._structureByteStride = sizeof(gpu::MeshletInstancePrimitiveInfo);
		device->createUnorderedAccessView(_primitiveInfoBuffer.getResource(), nullptr, &desc, _primitiveInfoUav._cpuHandle);

		desc._buffer._structureByteStride = sizeof(u32);
		device->createUnorderedAccessView(_primitiveInfoMeshInstanceIndexBuffer.getResource(), nullptr, &desc, _primitiveInfoMeshInstanceIndexUav._cpuHandle);

		desc._format = FORMAT_R32_TYPELESS;
		desc._buffer._structureByteStride = 0;
		desc._buffer._flags = BUFFER_UAV_FLAG_RAW;
		desc._buffer._numElements = INDIRECT_ARGUMENT_COUNTER_COUNT_MAX;
		device->createUnorderedAccessView(_infoCountBuffer.getResource(), nullptr, &desc, _infoCountUav._cpuHandle);
		device->createUnorderedAccessView(_infoCountBuffer.getResource(), nullptr, &desc, _infoCountCpuUav._cpuHandle);
	}
}

void InstancingResource::terminate() {
	_infoOffsetBuffer.terminate();
	_infoCountBuffer.terminate();
	_primitiveInfoBuffer.terminate();
	_primitiveInfoMeshInstanceIndexBuffer.terminate();

	DescriptorHeapAllocator* allocator = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	allocator->discardDescriptor(_infoOffsetSrv);
	allocator->discardDescriptor(_infoCountSrv);
	allocator->discardDescriptor(_infoCountUav);
	allocator->discardDescriptor(_primitiveInfoSrv);
	allocator->discardDescriptor(_primitiveInfoMeshInstanceIndexSrv);
	allocator->discardDescriptor(_primitiveInfoUav);
	allocator->discardDescriptor(_primitiveInfoMeshInstanceIndexUav);

	DescriptorHeapAllocator* cpuAllocator = GraphicsSystemImpl::Get()->getSrvCbvUavCpuDescriptorAllocator();
	cpuAllocator->discardDescriptor(_infoCountCpuUav);
}

void InstancingResource::update(const UpdateDesc& desc) {
	const MeshInstanceImpl* meshInstances = desc._meshInstances;
	u32 offsetUpdateStart = 0;
	u32 countMax = desc._countMax;
	bool* packMeshletFilterFlgas = new bool[INSTANCING_PER_SHADER_COUNT_MAX]();
	u16* infoCounts = new u16[INDIRECT_ARGUMENT_COUNTER_COUNT_MAX]();
	for (u32 meshInstanceIndex = 0; meshInstanceIndex < countMax; ++meshInstanceIndex) {
		const Mesh* mesh = meshInstances[meshInstanceIndex].getMesh();
		const MeshInfo* meshInfo = mesh->getMeshInfo();
		const gpu::SubMesh* subMeshes = mesh->getGpuSubMesh();
		const gpu::SubMeshInstance* subMeshInstances = meshInstances[meshInstanceIndex].getGpuSubMeshInstance(0);
		u32 subMeshStartOffset = meshInfo->_subMeshStartIndex;
		u32 subMeshCount = meshInfo->_totalSubMeshCount;
		for (u32 subMeshIndex = 0; subMeshIndex < subMeshCount; ++subMeshIndex) {
			const gpu::SubMesh* subMesh = &subMeshes[subMeshIndex];
			u32 subMeshGlobalIndex = subMeshStartOffset + subMeshIndex;
			u32 shaderSetIndex = subMeshInstances[subMeshIndex]._shaderSetIndex;
			u32 shaderSetOffset = shaderSetIndex * INSTANCING_PER_SHADER_COUNT_MAX;
			++infoCounts[shaderSetOffset + subMeshGlobalIndex];
			packMeshletFilterFlgas[subMeshGlobalIndex] = subMesh->_meshletCount <= desc._meshletThresholdUseAmplificationShader;
		}
	}

	for (u32 shaderSetIndex = 0; shaderSetIndex < gpu::SHADER_SET_COUNT_MAX; ++shaderSetIndex) {
		u32 shaderSetOffset = shaderSetIndex * INSTANCING_PER_SHADER_COUNT_MAX;
		u16 asMsIndirectArgumentCount = 0;
		u16 msIndirectArgumentCount = 0;
		for (u32 i = 0; i < INSTANCING_PER_SHADER_COUNT_MAX; ++i) {
			u16 infoCount = infoCounts[shaderSetOffset + i] > 0 ? 1 : 0;
			if (packMeshletFilterFlgas[i]) {
				msIndirectArgumentCount += infoCount;
				continue;
			}
			asMsIndirectArgumentCount += infoCount;
		}
		_msIndirectArgumentCounts[shaderSetIndex] = msIndirectArgumentCount;
		_asMsIndirectArgumentCounts[shaderSetIndex] = asMsIndirectArgumentCount;
	}

	u32* infoOffsets = new u32[INDIRECT_ARGUMENT_COUNTER_COUNT_MAX]();
	for (u32 i = 1; i < INDIRECT_ARGUMENT_COUNTER_COUNT_MAX; ++i) {
		u32 prevIndex = i - 1;
		infoOffsets[i] = infoOffsets[prevIndex] + infoCounts[prevIndex];
	}

	VramBufferUpdater* vramUpdater = GraphicsSystemImpl::Get()->getVramUpdater();
	u32* mapOffsets = vramUpdater->enqueueUpdate<u32>(&_infoOffsetBuffer, 0, INDIRECT_ARGUMENT_COUNTER_COUNT_MAX);
	memcpy(mapOffsets, infoOffsets, sizeof(u32)*INDIRECT_ARGUMENT_COUNTER_COUNT_MAX);

	delete[] infoCounts;
	delete[] infoOffsets;
	delete[] packMeshletFilterFlgas;
}

void InstancingResource::resetInfoCountBuffers(CommandList* commandList) {
	u32 clearValues[4] = {};
	GpuDescriptorHandle gpuDescriptor = _infoCountUav._gpuHandle;
	CpuDescriptorHandle cpuDescriptor = _infoCountCpuUav._cpuHandle;
	commandList->clearUnorderedAccessViewUint(gpuDescriptor, cpuDescriptor, _infoCountBuffer.getResource(), clearValues, 0, nullptr);
}

void InstancingResource::resourceBarriersGpuCullingToUAV(CommandList* commandList) {
	ResourceTransitionBarrier barriers[3] = {};
	barriers[0] = _infoCountBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_UNORDERED_ACCESS);
	barriers[1] = _primitiveInfoBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_UNORDERED_ACCESS);
	barriers[2] = _primitiveInfoMeshInstanceIndexBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_UNORDERED_ACCESS);
	commandList->transitionBarriers(barriers, LTN_COUNTOF(barriers));
}

void InstancingResource::resetResourceGpuCullingBarriers(CommandList* commandList) {
	ResourceTransitionBarrier barriers[3] = {};
	barriers[0] = _infoCountBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	barriers[1] = _primitiveInfoBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	barriers[2] = _primitiveInfoMeshInstanceIndexBuffer.getAndUpdateTransitionBarrier(RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	commandList->transitionBarriers(barriers, LTN_COUNTOF(barriers));
}

void GpuCullingResource::initialize() {
	Device* device = GraphicsSystemImpl::Get()->getDevice();
	DescriptorHeapAllocator* allocator = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	DescriptorHeapAllocator* cpuAllocator = GraphicsSystemImpl::Get()->getSrvCbvUavCpuDescriptorAllocator();

	// culling result buffers
	{
		GpuBufferDesc desc = {};
		desc._sizeInByte = sizeof(gpu::GpuCullingResult);
		desc._initialState = RESOURCE_STATE_UNORDERED_ACCESS;
		desc._flags = RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		desc._device = device;
		_gpuCullingResultBuffer.initialize(desc);
		_gpuCullingResultBuffer.setDebugName("Gpu Culling Result");

		desc._sizeInByte = sizeof(gpu::AmplificationCullingResult);
		_amplificationCullingResultBuffer.initialize(desc);
		_amplificationCullingResultBuffer.setDebugName("Amplification Culling Result");
	}

	// current lod level buffers
	{
		GpuBufferDesc desc = {};
		desc._sizeInByte = sizeof(u32) * Scene::MESH_INSTANCE_COUNT_MAX;
		desc._initialState = RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		desc._flags = RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		desc._device = device;
		_currentLodLevelBuffer.initialize(desc);
		_currentLodLevelBuffer.setDebugName("Current Lod Levels");
	}

	// culling result readback buffers
	{
		GpuBufferDesc desc = {};
		desc._device = device;
		desc._sizeInByte = sizeof(gpu::GpuCullingResult) * BACK_BUFFER_COUNT;
		desc._initialState = RESOURCE_STATE_COPY_DEST;
		desc._heapType = HEAP_TYPE_READBACK;
		_gpuCullingResultReadbackBuffer.initialize(desc);
		_gpuCullingResultReadbackBuffer.setDebugName("Culling Result Readback");

		desc._sizeInByte = sizeof(gpu::AmplificationCullingResult) * BACK_BUFFER_COUNT;
		_amplificationCullingResultReadbackBuffer.initialize(desc);
		_amplificationCullingResultReadbackBuffer.setDebugName("Amplification Culling Result Readback");
	}

	// culling result uav
	{
		_gpuCullingResultUavHandle = allocator->allocateDescriptors(1);
		_gpuCullingResultCpuUavHandle = cpuAllocator->allocateDescriptors(1);
		_amplificationCullingResultUavHandle = allocator->allocateDescriptors(1);
		_amplificationCullingResultCpuUavHandle = cpuAllocator->allocateDescriptors(1);

		UnorderedAccessViewDesc desc = {};
		desc._format = FORMAT_R32_TYPELESS;
		desc._viewDimension = UAV_DIMENSION_BUFFER;
		desc._buffer._firstElement = 0;
		desc._buffer._numElements = sizeof(gpu::GpuCullingResult) / sizeof(u32);
		desc._buffer._flags = BUFFER_UAV_FLAG_RAW;
		device->createUnorderedAccessView(_gpuCullingResultBuffer.getResource(), nullptr, &desc, _gpuCullingResultUavHandle._cpuHandle);
		device->createUnorderedAccessView(_gpuCullingResultBuffer.getResource(), nullptr, &desc, _gpuCullingResultCpuUavHandle._cpuHandle);

		desc._buffer._numElements = sizeof(gpu::AmplificationCullingResult) / sizeof(u32);
		device->createUnorderedAccessView(_amplificationCullingResultBuffer.getResource(), nullptr, &desc, _amplificationCullingResultUavHandle._cpuHandle);
		device->createUnorderedAccessView(_amplificationCullingResultBuffer.getResource(), nullptr, &desc, _amplificationCullingResultCpuUavHandle._cpuHandle);
	}

	// current lod level uav
	{
		_currentLodLevelUav = allocator->allocateDescriptors(1);

		UnorderedAccessViewDesc desc = {};
		desc._format = FORMAT_R32_TYPELESS;
		desc._viewDimension = UAV_DIMENSION_BUFFER;
		desc._buffer._firstElement = 0;
		desc._buffer._numElements = Scene::MESH_INSTANCE_COUNT_MAX;
		desc._buffer._flags = BUFFER_UAV_FLAG_RAW;
		device->createUnorderedAccessView(_currentLodLevelBuffer.getResource(), nullptr, &desc, _currentLodLevelUav._cpuHandle);
	}

	// current lod level srv
	{
		_currentLodLevelSrv = allocator->allocateDescriptors(1);
		ShaderResourceViewDesc desc = {};
		desc._format = FORMAT_R32_TYPELESS;
		desc._viewDimension = SRV_DIMENSION_BUFFER;
		desc._buffer._numElements = Scene::MESH_INSTANCE_COUNT_MAX;
		desc._buffer._flags = BUFFER_SRV_FLAG_RAW;
		device->createShaderResourceView(_currentLodLevelBuffer.getResource(), &desc, _currentLodLevelSrv._cpuHandle);
	}

	// build hiz textures
	{
		_hizDepthTextureUav = allocator->allocateDescriptors(gpu::HIERACHICAL_DEPTH_COUNT);
		_hizDepthTextureSrv = allocator->allocateDescriptors(gpu::HIERACHICAL_DEPTH_COUNT);

		u32 incrimentSize = allocator->getIncrimentSize();
		Application* app = ApplicationSystem::Get()->getApplication();
		u32 powScale = 1;
		for (u32 i = 0; i < gpu::HIERACHICAL_DEPTH_COUNT; ++i) {
			powScale *= 2;
			u32 screenWidth = app->getScreenWidth() / powScale;
			u32 screenHeight = app->getScreenHeight() / powScale;
			screenWidth += screenWidth % 2;
			screenHeight += screenHeight % 2;

			GpuTextureDesc desc = {};
			desc._device = device;
			desc._format = FORMAT_R32_FLOAT;
			desc._width = screenWidth;
			desc._height = screenHeight;
			desc._flags = RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			desc._initialState = RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			_hizDepthTextures[i].initialize(desc);

			char debugName[64];
			sprintf_s(debugName, "Hiz [%-2d] %-4d x %-4d", i, screenWidth, screenHeight);
			_hizDepthTextures[i].setDebugName(debugName);
		}

		for (u32 i = 0; i < gpu::HIERACHICAL_DEPTH_COUNT; ++i) {
			CpuDescriptorHandle uavHandle = _hizDepthTextureUav._cpuHandle + static_cast<u64>(incrimentSize) * i;
			UnorderedAccessViewDesc uavDesc = {};
			uavDesc._format = FORMAT_R32_FLOAT;
			uavDesc._viewDimension = UAV_DIMENSION_TEXTURE2D;
			uavDesc._texture2D._mipSlice = 0;
			uavDesc._texture2D._planeSlice = 0;
			device->createUnorderedAccessView(_hizDepthTextures[i].getResource(), nullptr, &uavDesc, uavHandle);

			CpuDescriptorHandle srvHandle = _hizDepthTextureSrv._cpuHandle + static_cast<u64>(incrimentSize) * i;
			device->createShaderResourceView(_hizDepthTextures[i].getResource(), nullptr, srvHandle);
		}
	}

	// build hiz constant buffers
	{
		GpuBuffer& buffer = _hizInfoConstantBuffer[0];
		GpuBufferDesc desc = {};
		desc._sizeInByte = GetConstantBufferAligned(sizeof(HizInfoConstant));
		desc._initialState = RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		desc._device = device;
		buffer.initialize(desc);
		buffer.setDebugName("Hiz Info Constant 0");
	}

	{
		GpuBuffer& buffer = _hizInfoConstantBuffer[1];
		GpuBufferDesc desc = {};
		desc._sizeInByte = GetConstantBufferAligned(sizeof(HizInfoConstant));
		desc._initialState = RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		desc._device = device;
		buffer.initialize(desc);
		buffer.setDebugName("Hiz Info Constant 1");
	}

	// build hiz cbv descriptors
	{
		for (u32 i = 0; i < LTN_COUNTOF(_hizInfoConstantBuffer); ++i) {
			_hizInfoConstantCbv[i] = allocator->allocateDescriptors(1);
			device->createConstantBufferView(_hizInfoConstantBuffer[i].getConstantBufferViewDesc(), _hizInfoConstantCbv[i]._cpuHandle);
		}
	}
}

void GpuCullingResource::terminate() {
	_gpuCullingResultBuffer.terminate();
	_currentLodLevelBuffer.terminate();
	_gpuCullingResultReadbackBuffer.terminate();
	_amplificationCullingResultBuffer.terminate();
	_amplificationCullingResultReadbackBuffer.terminate();

	for (u32 i = 0; i < LTN_COUNTOF(_hizInfoConstantBuffer); ++i) {
		_hizInfoConstantBuffer[i].terminate();
	}

	for (u32 i = 0; i < gpu::HIERACHICAL_DEPTH_COUNT; ++i) {
		_hizDepthTextures[i].terminate();
	}

	DescriptorHeapAllocator* allocator = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	allocator->discardDescriptor(_gpuCullingResultUavHandle);
	allocator->discardDescriptor(_amplificationCullingResultUavHandle);
	allocator->discardDescriptor(_currentLodLevelUav);
	allocator->discardDescriptor(_currentLodLevelSrv);
	allocator->discardDescriptor(_hizDepthTextureUav);
	allocator->discardDescriptor(_hizDepthTextureSrv);
	for (u32 i = 0; i < LTN_COUNTOF(_hizInfoConstantBuffer); ++i) {
		allocator->discardDescriptor(_hizInfoConstantCbv[i]);
	}

	DescriptorHeapAllocator* cpuAllocator = GraphicsSystemImpl::Get()->getSrvCbvUavCpuDescriptorAllocator();
	cpuAllocator->discardDescriptor(_gpuCullingResultCpuUavHandle);
	cpuAllocator->discardDescriptor(_amplificationCullingResultCpuUavHandle);
}

void GpuCullingResource::update(const ViewInfo* viewInfo) {
	const ViewConstantInfo& viewConstantInfo = viewInfo->_mainViewConstantInfo;
	VramBufferUpdater* vramUpdater = GraphicsSystemImpl::Get()->getVramUpdater();
	Application* app = ApplicationSystem::Get()->getApplication();
	{
		HizInfoConstant* mapConstant = vramUpdater->enqueueUpdate<HizInfoConstant>(&_hizInfoConstantBuffer[0], 0);
		mapConstant->_inputDepthWidth = app->getScreenWidth();
		mapConstant->_inputDepthHeight = app->getScreenHeight();
		mapConstant->_nearClip = viewConstantInfo._nearClip;
		mapConstant->_farClip = viewConstantInfo._farClip;
		mapConstant->_inputBitDepth = UINT32_MAX;
	}

	{
		ResourceDesc hizLevel3Desc = getHizTextureResourceDesc(3);
		HizInfoConstant* mapConstant = vramUpdater->enqueueUpdate<HizInfoConstant>(&_hizInfoConstantBuffer[1], 0);
		mapConstant->_inputDepthWidth = static_cast<u32>(hizLevel3Desc._width);
		mapConstant->_inputDepthHeight = static_cast<u32>(hizLevel3Desc._height);
		mapConstant->_nearClip = viewConstantInfo._nearClip;
		mapConstant->_farClip = viewConstantInfo._farClip;
		mapConstant->_inputBitDepth = UINT16_MAX;
	}
}

void GpuCullingResource::debugDrawHiz() {
	DescriptorHeapAllocator* descriptorHeapAllocater = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	u32 incrimentSize = descriptorHeapAllocater->getIncrimentSize();

	ResourceDesc firstDesc = _hizDepthTextures[0].getResourceDesc();
	f32 aspectRate = firstDesc._width / static_cast<f32>(firstDesc._height);

	for (u32 i = 0; i < gpu::HIERACHICAL_DEPTH_COUNT; ++i) {
		ResourceDesc desc = _hizDepthTextures[i].getResourceDesc();
		DebugGui::Text("[%d] width:%-4d height:%-4d", i, desc._width, desc._height);
		DebugGui::Image(_hizDepthTextureSrv._gpuHandle + incrimentSize * i, Vector2(200 * aspectRate, 200),
			Vector2(0, 0), Vector2(1, 1), Color4::WHITE, Color4::BLACK, DebugGui::COLOR_CHANNEL_FILTER_R, Vector2(0.95f, 1), DebugGui::TEXTURE_SAMPLE_TYPE_POINT);
	}
}

#if ENABLE_MULTI_INDIRECT_DRAW
void MultiDrawInstancingResource::initialize() {
	Device* device = GraphicsSystemImpl::Get()->getDevice();
	// buffers
	{
		GpuBufferDesc desc = {};
		desc._sizeInByte = Scene::SUB_MESH_INSTANCE_COUNT_MAX * sizeof(gpu::SubMeshInstance);
		desc._initialState = RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		desc._device = device;

		_indirectArgumentOffsetBuffer.initialize(desc);
		_indirectArgumentOffsetBuffer.setDebugName("Multi Draw Indirect Argument Offsets");
	}

	// srv
	{
		DescriptorHeapAllocator* allocator = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
		ShaderResourceViewDesc desc = {};
		desc._format = FORMAT_UNKNOWN;
		desc._viewDimension = SRV_DIMENSION_BUFFER;
		desc._buffer._firstElement = 0;
		desc._buffer._flags = BUFFER_SRV_FLAG_NONE;
		desc._buffer._numElements = gpu::SHADER_SET_COUNT_MAX;
		desc._buffer._structureByteStride = sizeof(u32);

		_indirectArgumentOffsetSrv = allocator->allocateDescriptors(1);
		device->createShaderResourceView(_indirectArgumentOffsetBuffer.getResource(), &desc, _indirectArgumentOffsetSrv._cpuHandle);
	}
}

void MultiDrawInstancingResource::terminate() {
	_indirectArgumentOffsetBuffer.terminate();

	DescriptorHeapAllocator* allocator = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	allocator->discardDescriptor(_indirectArgumentOffsetSrv);
}

void MultiDrawInstancingResource::update(const UpdateDesc& desc) {
	u32 countMax = desc._countMax;
	memset(_indirectArgumentCounts, 0, sizeof(u32) * gpu::SHADER_SET_COUNT_MAX);

	const gpu::SubMeshInstance* subMeshInstances = desc._subMeshInstances;
	u32 subMeshInstanceCount = desc._countMax;
	for (u32 subMeshInstanceIndex = 0; subMeshInstanceIndex < subMeshInstanceCount; ++subMeshInstanceIndex) {
		u32 shaderSetIndex = subMeshInstances[subMeshInstanceIndex]._shaderSetIndex;
		++_indirectArgumentCounts[shaderSetIndex];
	}

	for (u32 shaderSetIndex = 1; shaderSetIndex < gpu::SHADER_SET_COUNT_MAX; ++shaderSetIndex) {
		u32 prevShaderIndex = shaderSetIndex - 1;
		_indirectArgumentOffsets[shaderSetIndex] = _indirectArgumentOffsets[prevShaderIndex] + _indirectArgumentCounts[prevShaderIndex];
	}

	VramBufferUpdater* vramUpdater = GraphicsSystemImpl::Get()->getVramUpdater();
	u32* mapIndirectArgumentOffsets = vramUpdater->enqueueUpdate<u32>(&_indirectArgumentOffsetBuffer, 0, gpu::SHADER_SET_COUNT_MAX);
	memcpy(mapIndirectArgumentOffsets, _indirectArgumentOffsets, sizeof(u32) * gpu::SHADER_SET_COUNT_MAX);
}
#endif

void BuildIndirectArgumentResource::initialize() {
	Device* device = GraphicsSystemImpl::Get()->getDevice();
	GpuBufferDesc desc = {};
	desc._device = device;
	desc._sizeInByte = GetConstantBufferAligned(sizeof(Constant));
	desc._initialState = RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	_constantBuffer.initialize(desc);
	_constantBuffer.setDebugName("Build Indirect Argument Constant");

	DescriptorHeapAllocator* allocator = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	_constantCbv = allocator->allocateDescriptors(1);
	device->createConstantBufferView(_constantBuffer.getConstantBufferViewDesc(), _constantCbv._cpuHandle);
}

void BuildIndirectArgumentResource::terminate() {
	_constantBuffer.terminate();

	DescriptorHeapAllocator* allocator = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	allocator->discardDescriptor(_constantCbv);
}

void BuildIndirectArgumentResource::update(const UpdateDesc& desc) {
	VramBufferUpdater* vramUpdater = GraphicsSystemImpl::Get()->getVramUpdater();
	Constant* constant = vramUpdater->enqueueUpdate<Constant>(&_constantBuffer, 0, 1);
	constant->_packedMeshletCount = desc._packedMeshletCount;
}
