#include <MeshRenderer/impl/MeshResourceManager.h>
#include <GfxCore/impl/GraphicsSystemImpl.h>
#include <GfxCore/impl/QueryHeapSystem.h>
#include <DebugRenderer/DebugRendererSystem.h>

void MeshResourceManager::initialize() {
	Device* device = GraphicsSystemImpl::Get()->getDevice();

	// buffers
	{
		GpuBufferDesc desc = {};
		desc._sizeInByte = VERTEX_COUNT_MAX * sizeof(VertexPosition);
		desc._initialState = RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		desc._device = device;
		_positionVertexBuffer.initialize(desc);
		_positionVertexBuffer.setDebugName("Position Vertex");

		desc._sizeInByte = VERTEX_COUNT_MAX * sizeof(VertexNormalTangent);
		_normalTangentVertexBuffer.initialize(desc);
		_normalTangentVertexBuffer.setDebugName("Normal Tangent Vertex");

		desc._sizeInByte = VERTEX_COUNT_MAX * sizeof(VertexTexcoord);
		_texcoordVertexBuffer.initialize(desc);
		_texcoordVertexBuffer.setDebugName("Texcoord Vertex");

		desc._sizeInByte = INDEX_COUNT_MAX * sizeof(u32);
		_vertexIndexBuffer.initialize(desc);
		_vertexIndexBuffer.setDebugName("Index Vertex");

		desc._sizeInByte = INDEX_COUNT_MAX * sizeof(u32);
		_primitiveBuffer.initialize(desc);
		_primitiveBuffer.setDebugName("Primitive");

#if ENABLE_CLASSIC_VERTEX
		desc._sizeInByte = INDEX_COUNT_MAX * sizeof(u32);
		_classicIndexBuffer.initialize(desc);
		_classicIndexBuffer.setDebugName("Classic Index");
#endif
		desc._sizeInByte = SUB_MESH_COUNT_MAX * sizeof(gpu::SubMeshDrawInfo);
		_subMeshDrawInfoBuffer.initialize(desc);
		_subMeshDrawInfoBuffer.setDebugName("SubMesh Draw Info");

		desc._sizeInByte = MESH_COUNT_MAX * sizeof(gpu::Mesh);
		_meshBuffer.initialize(desc);
		_meshBuffer.setDebugName("Mesh");

		desc._sizeInByte = LOD_MESH_COUNT_MAX * sizeof(gpu::LodMesh);
		_lodMeshBuffer.initialize(desc);
		_lodMeshBuffer.setDebugName("LodMesh");

		desc._sizeInByte = SUB_MESH_COUNT_MAX * sizeof(gpu::SubMesh);
		_subMeshBuffer.initialize(desc);
		_subMeshBuffer.setDebugName("SubMesh");

		desc._sizeInByte = MESHLET_COUNT_MAX * sizeof(gpu::Meshlet);
		_meshletBuffer.initialize(desc);
		_meshletBuffer.setDebugName("Meshlet");

		desc._sizeInByte = MESHLET_COUNT_MAX * sizeof(gpu::MeshletPrimitiveInfo);
		_meshletPrimitiveInfoBuffer.initialize(desc);
		_meshletPrimitiveInfoBuffer.setDebugName("Meshlet Primitive Info");
	}

	_meshes.initialize(MESH_COUNT_MAX);
	_lodMeshes.initialize(LOD_MESH_COUNT_MAX);
	_subMeshes.initialize(SUB_MESH_COUNT_MAX);
	_meshlets.initialize(MESHLET_COUNT_MAX);

	_vertexPositionBinaryHeaders.initialize(VERTEX_COUNT_MAX);
	_indexBinaryHeaders.initialize(INDEX_COUNT_MAX);
	_primitiveBinaryHeaders.initialize(PRIMITIVE_COUNT_MAX);
#if ENABLE_CLASSIC_VERTEX
	_classicIndexBinaryHeaders.initialize(INDEX_COUNT_MAX);
#endif

	// descriptors
	{
		DescriptorHeapAllocator* allocater = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
		_meshSrv = allocater->allocateDescriptors(5);
		_vertexSrv = allocater->allocateDescriptors(5);
		_subMeshDrawInfoSrv = allocater->allocateDescriptors(1);
		u64 incrimentSize = static_cast<u64>(allocater->getIncrimentSize());

		ShaderResourceViewDesc desc = {};
		desc._format = FORMAT_UNKNOWN;
		desc._viewDimension = SRV_DIMENSION_BUFFER;
		desc._buffer._firstElement = 0;
		desc._buffer._flags = BUFFER_SRV_FLAG_NONE;

		// mesh
		{
			CpuDescriptorHandle handle = _meshSrv._cpuHandle;

			desc._buffer._numElements = MESH_COUNT_MAX;
			desc._buffer._structureByteStride = sizeof(gpu::Mesh);
			device->createShaderResourceView(_meshBuffer.getResource(), &desc, handle + incrimentSize * 0);

			desc._buffer._numElements = LOD_MESH_COUNT_MAX;
			desc._buffer._structureByteStride = sizeof(gpu::LodMesh);
			device->createShaderResourceView(_lodMeshBuffer.getResource(), &desc, handle + incrimentSize * 1);

			desc._buffer._numElements = SUB_MESH_COUNT_MAX;
			desc._buffer._structureByteStride = sizeof(gpu::SubMesh);
			device->createShaderResourceView(_subMeshBuffer.getResource(), &desc, handle + incrimentSize * 2);

			desc._buffer._numElements = MESHLET_COUNT_MAX;
			desc._buffer._structureByteStride = sizeof(gpu::Meshlet);
			device->createShaderResourceView(_meshletBuffer.getResource(), &desc, handle + incrimentSize * 3);

			desc._buffer._numElements = MESHLET_COUNT_MAX;
			desc._buffer._structureByteStride = sizeof(gpu::MeshletPrimitiveInfo);
			device->createShaderResourceView(_meshletPrimitiveInfoBuffer.getResource(), &desc, handle + incrimentSize * 4);

			desc._buffer._numElements = SUB_MESH_COUNT_MAX;
			desc._buffer._structureByteStride = sizeof(gpu::SubMeshDrawInfo);
			device->createShaderResourceView(_subMeshDrawInfoBuffer.getResource(), &desc, _subMeshDrawInfoSrv._cpuHandle);
		}

		// vertex
		{
			CpuDescriptorHandle handle = _vertexSrv._cpuHandle;

			desc._buffer._numElements = VERTEX_COUNT_MAX;
			desc._buffer._structureByteStride = sizeof(u32);
			device->createShaderResourceView(_vertexIndexBuffer.getResource(), &desc, handle + incrimentSize * 0);
			device->createShaderResourceView(_primitiveBuffer.getResource(), &desc, handle + incrimentSize * 1);

			desc._buffer._structureByteStride = sizeof(VertexPosition);
			device->createShaderResourceView(_positionVertexBuffer.getResource(), &desc, handle + incrimentSize * 2);

			desc._buffer._structureByteStride = sizeof(VertexNormalTangent);
			device->createShaderResourceView(_normalTangentVertexBuffer.getResource(), &desc, handle + incrimentSize * 3);

			desc._buffer._structureByteStride = sizeof(VertexTexcoord);
			device->createShaderResourceView(_texcoordVertexBuffer.getResource(), &desc, handle + incrimentSize * 4);
		}
	}

	MeshDesc desc = {};
	desc._filePath = "common/box.mesh";
	_defaultCube = createMesh(desc);
}

void MeshResourceManager::update() {
	u32 meshCount = _meshes.getResarveCount();
	for (u32 meshIndex = 0; meshIndex < meshCount; ++meshIndex) {
		if (_assetStateFlags[meshIndex] == ASSET_STATE_REQUEST_LOAD) {
			loadMesh(meshIndex);
		}
	}
}

void MeshResourceManager::processDeletion() {
	u32 meshCount = _meshes.getResarveCount();
	for (u32 meshIndex = 0; meshIndex < meshCount; ++meshIndex) {
		if (_meshStateFlags[meshIndex] & MESH_FLAG_STATE_REQUEST_DESTROY) {
			deleteMesh(meshIndex);
		}
	}
}

void MeshResourceManager::terminate() {
	_positionVertexBuffer.terminate();
	_normalTangentVertexBuffer.terminate();
	_texcoordVertexBuffer.terminate();
	_vertexIndexBuffer.terminate();
	_primitiveBuffer.terminate();
	_meshBuffer.terminate();
	_lodMeshBuffer.terminate();
	_subMeshBuffer.terminate();
	_meshletBuffer.terminate();
	_meshletPrimitiveInfoBuffer.terminate();
	_subMeshDrawInfoBuffer.terminate();

#if ENABLE_CLASSIC_VERTEX
	_classicIndexBuffer.terminate();
	_classicIndexBinaryHeaders.terminate();
#endif

	LTN_ASSERT(_meshes.getInstanceCount() == 0);
	LTN_ASSERT(_lodMeshes.getInstanceCount() == 0);
	LTN_ASSERT(_subMeshes.getInstanceCount() == 0);

	_meshes.terminate();
	_lodMeshes.terminate();
	_subMeshes.terminate();
	_meshlets.terminate();

	_vertexPositionBinaryHeaders.terminate();
	_indexBinaryHeaders.terminate();
	_primitiveBinaryHeaders.terminate();

	DescriptorHeapAllocator* allocater = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator();
	allocater->discardDescriptor(_meshSrv);
	allocater->discardDescriptor(_vertexSrv);
	allocater->discardDescriptor(_subMeshDrawInfoSrv);
}

void MeshResourceManager::terminateDefaultResources() {
	_defaultCube->requestToDelete();
}

void MeshResourceManager::drawDebugGui() {
	u32 meshCount = _meshes.getResarveCount();
	DebugGui::Text("Total:%3d", meshCount);
	DebugGui::Columns(2, "tree", true);
	constexpr f32 MESH_NAME_WIDTH = 320;
	DebugGui::SetColumnWidth(0, MESH_NAME_WIDTH);
	for (u32 x = 0; x < meshCount; x++) {
		if (_fileHashes[x] == 0) {
			DebugGui::TextDisabled("Disabled");
			DebugGui::NextColumn();
			DebugGui::TextDisabled("Disabled");
			DebugGui::NextColumn();
			continue;
		}
		const MeshInfo& meshInfo = _meshInfos[x];
		const SubMeshInfo* subMeshInfos = &_subMeshInfos[meshInfo._subMeshStartIndex];
		const DebugMeshInfo& debugMeshInfo = _debugMeshInfo[x];
		bool open1 = DebugGui::TreeNode(static_cast<s32>(x), "%-3d: %-20s", x, debugMeshInfo._filePath);
		Color4 lodCountTextColor = open1 ? Color4::GREEN : Color4::WHITE;
		DebugGui::NextColumn();
		DebugGui::TextColored(lodCountTextColor, "Lod Count:%-2d", meshInfo._totalLodMeshCount);
		DebugGui::NextColumn();
		if (!open1) {
			continue;
		}

		// mesh info
		DebugGui::Separator();
		u32 subMeshIndex = 0;
		for (u32 y = 0; y < meshInfo._totalLodMeshCount; y++) {
			bool open2 = DebugGui::TreeNode(static_cast<s32>(y), "Lod %2d", y);
			u32 subMeshCount = _lodMeshes[meshInfo._lodMeshStartIndex + y]._subMeshCount;
			Color4 subMeshCountTextColor = open2 ? Color4::GREEN : Color4::WHITE;
			DebugGui::NextColumn();
			DebugGui::TextColored(subMeshCountTextColor, "Sub Mesh Count:%2d", subMeshCount);
			DebugGui::NextColumn();
			if (!open2) {
				continue;
			}

			DebugGui::Separator();

			constexpr char lodFormat[] = "%-14s:%-7d";
			DebugGui::Text(lodFormat, "Vertex Count", meshInfo._vertexCount);
			DebugGui::Text(lodFormat, "Index Count", meshInfo._indexCount);

			constexpr char boundsFormat[] = "%-14s:[%5.1f][%5.1f][%5.1f]";
			DebugGui::Text(boundsFormat, "Bounds Min", meshInfo._boundsMin._x, meshInfo._boundsMin._y, meshInfo._boundsMin._z);
			DebugGui::Text(boundsFormat, "Bounds Max", meshInfo._boundsMax._x, meshInfo._boundsMax._y, meshInfo._boundsMax._z);

			for (u32 z = 0; z < subMeshCount; z++) {
				bool open3 = DebugGui::TreeNode(static_cast<s32>(z), "Sub Mesh %2d", z);
				Color4 meshletCountTextColor = open3 ? Color4::GREEN : Color4::WHITE;
				u32 subMeshLocalIndex = subMeshIndex + z;
				u32 meshletCount = subMeshInfos[z]._meshletCount;
				DebugGui::NextColumn();
				DebugGui::TextColored(meshletCountTextColor, "Meshlet Count:%2d", meshletCount);
				DebugGui::NextColumn();

				if (!open3) {
					continue;
				}

				DebugGui::Separator();
				u32 materialSlotIndex = _subMeshes[meshInfo._subMeshStartIndex + subMeshLocalIndex]._materialSlotIndex;
				DebugGui::Text("%10s:%2d", "Slot Index", materialSlotIndex);

				for (u32 w = 0; w < meshletCount; w++) {
					DebugGui::Text("Meshlet %2d", w);
				}

				//メッシュレット情報
				DebugGui::TreePop();
			}
			subMeshIndex += subMeshCount;
			DebugGui::TreePop();
		}

		DebugGui::TreePop();
	}
	DebugGui::Columns(1);

	DebugGui::EndTabItem();
}

void MeshResourceManager::loadMesh(u32 meshIndex) {
	LTN_ASSERT(_assetStateFlags[meshIndex] == ASSET_STATE_REQUEST_LOAD);

	Asset& asset = _meshAssets[meshIndex];
	asset.openFile(asset.getFilePath());
	FILE* fin = asset.getFile();
	fseek(fin, asset.getFileOffsetInByte(), SEEK_SET);

	VramBufferUpdater* vramUpdater = GraphicsSystemImpl::Get()->getVramUpdater();
	const MeshInfo& meshInfo = _meshInfos[meshIndex];

	// 頂点・インデックスバイナリを格納
	u32 vertexCount = meshInfo._vertexCount;
	u32 indexCount = meshInfo._indexCount;
	u32 primitiveCount = meshInfo._primitiveCount;
	u32 vertexBinaryIndex = _vertexPositionBinaryHeaders.request(vertexCount);
	u32 indexBinaryIndex = _indexBinaryHeaders.request(indexCount);
	u32 primitiveBinaryIndex = _primitiveBinaryHeaders.request(primitiveCount);

	// Meshlet
	{
		u32 startIndex = meshInfo._meshletStartIndex;
		u32 count = meshInfo._totalMeshletCount;
		gpu::MeshletPrimitiveInfo* meshletPrimitiveInfos = vramUpdater->enqueueUpdate<gpu::MeshletPrimitiveInfo>(&_meshletPrimitiveInfoBuffer, sizeof(gpu::MeshletPrimitiveInfo) * startIndex, count);
		fread_s(meshletPrimitiveInfos, sizeof(gpu::MeshletPrimitiveInfo) * count, sizeof(gpu::MeshletPrimitiveInfo), count, fin);
		
		gpu::Meshlet* meshlets = vramUpdater->enqueueUpdate<gpu::Meshlet>(&_meshletBuffer, sizeof(gpu::Meshlet) * startIndex, count);
		fread_s(meshlets, sizeof(gpu::Meshlet) * count, sizeof(gpu::Meshlet), count, fin);
	}

	// プリミティブ
	u32* primitivePtr = nullptr;
	{
		u32 dstOffset = sizeof(u32) * primitiveBinaryIndex;
		primitivePtr = vramUpdater->enqueueUpdate<u32>(&_primitiveBuffer, dstOffset, primitiveCount);
		fread_s(primitivePtr, sizeof(u32) * primitiveCount, sizeof(u32), primitiveCount, fin);
	}

	// 頂点インデックス
	u32* indexPtr = nullptr;
	{
		u32 dstOffset = sizeof(u32) * indexBinaryIndex;
		indexPtr = vramUpdater->enqueueUpdate<u32>(&_vertexIndexBuffer, dstOffset, indexCount);
		fread_s(indexPtr, sizeof(u32) * indexCount, sizeof(u32), indexCount, fin);
	}

	// 座標
	{
		u32 dstOffset = sizeof(VertexPosition) * vertexBinaryIndex;
		VertexPosition* vertexPtr = vramUpdater->enqueueUpdate<VertexPosition>(&_positionVertexBuffer, dstOffset, vertexCount);
		fread_s(vertexPtr, sizeof(VertexPosition) * vertexCount, sizeof(VertexPosition), vertexCount, fin);
	}

	// 法線 / 接線
	{
		u32 dstOffset = sizeof(VertexNormalTangent) * vertexBinaryIndex;
		VertexNormalTangent* normalPtr = vramUpdater->enqueueUpdate<VertexNormalTangent>(&_normalTangentVertexBuffer, dstOffset, vertexCount);
		fread_s(normalPtr, sizeof(VertexNormalTangent) * vertexCount, sizeof(VertexNormalTangent), vertexCount, fin);
	}

	// UV
	{
		u32 dstOffset = sizeof(VertexTexcoord) * vertexBinaryIndex;
		VertexTexcoord* texcoordPtr = vramUpdater->enqueueUpdate<VertexTexcoord>(&_texcoordVertexBuffer, dstOffset, vertexCount);
		fread_s(texcoordPtr, sizeof(VertexTexcoord) * vertexCount, sizeof(VertexTexcoord), vertexCount, fin);
	}

#if ENABLE_CLASSIC_VERTEX
	u32 classicIndexCount = meshInfo._classicIndexCount;
	u32 classicIndex = _classicIndexBinaryHeaders.request(classicIndexCount);
	// Classic Index
	{
		u32 dstOffset = sizeof(u32) * classicIndex;
		u32* classicIndexPtr = vramUpdater->enqueueUpdate<u32>(&_classicIndexBuffer, dstOffset, classicIndexCount);
		fread_s(classicIndexPtr, sizeof(u32) * classicIndexCount, sizeof(u32), classicIndexCount, fin);
	}

	SubMeshInfo* subMeshInfos = &_subMeshInfos[meshInfo._subMeshStartIndex];
	for (u32 subMeshIndex = 0; subMeshIndex < meshInfo._totalSubMeshCount; ++subMeshIndex) {
		SubMeshInfo& info = subMeshInfos[subMeshIndex];
		info._classiciIndexOffset += classicIndex;
	}
#endif

	MeshInfo& updateMeshInfo = _meshInfos[meshIndex];
	updateMeshInfo._vertexBinaryIndex = vertexBinaryIndex;
	updateMeshInfo._indexBinaryIndex = indexBinaryIndex;
	updateMeshInfo._primitiveBinaryIndex = primitiveBinaryIndex;
#if ENABLE_CLASSIC_VERTEX
	updateMeshInfo._classicIndexBinaryIndex = classicIndex;
#endif

	gpu::Mesh& mesh = _meshes[meshIndex];
	mesh._stateFlags = gpu::MESH_STATE_LOADED;

	for (u32 lodMeshLocalIndex = 0; lodMeshLocalIndex < meshInfo._totalLodMeshCount; ++lodMeshLocalIndex) {
		u32 lodMeshIndex = meshInfo._lodMeshStartIndex + lodMeshLocalIndex;
		gpu::LodMesh& lodMesh = _lodMeshes[lodMeshIndex];
		lodMesh._vertexOffset += vertexBinaryIndex;
		lodMesh._vertexIndexOffset += indexBinaryIndex;
		lodMesh._primitiveOffset += primitiveBinaryIndex;
	}

	// データVramアップロード
	gpu::SubMesh* subMeshes = vramUpdater->enqueueUpdate<gpu::SubMesh>(&_subMeshBuffer, sizeof(gpu::SubMesh) * meshInfo._subMeshStartIndex, meshInfo._totalSubMeshCount);
	gpu::LodMesh* lodMeshes = vramUpdater->enqueueUpdate<gpu::LodMesh>(&_lodMeshBuffer, sizeof(gpu::LodMesh) * meshInfo._lodMeshStartIndex, meshInfo._totalLodMeshCount);
	gpu::Mesh* meshes = vramUpdater->enqueueUpdate<gpu::Mesh>(&_meshBuffer, sizeof(gpu::Mesh) * meshIndex);
	memcpy(subMeshes, &_subMeshes[meshInfo._subMeshStartIndex], sizeof(gpu::SubMesh) * meshInfo._totalSubMeshCount);
	memcpy(lodMeshes, &_lodMeshes[meshInfo._lodMeshStartIndex], sizeof(gpu::LodMesh) * meshInfo._totalLodMeshCount);
	memcpy(meshes, &_meshes[meshIndex], sizeof(gpu::Mesh));

	gpu::SubMeshDrawInfo* subMeshDrawInfos = vramUpdater->enqueueUpdate<gpu::SubMeshDrawInfo>(&_subMeshDrawInfoBuffer, sizeof(gpu::SubMeshDrawInfo) * meshInfo._subMeshStartIndex, meshInfo._totalSubMeshCount);
	for (u32 subMeshIndex = 0; subMeshIndex < meshInfo._totalSubMeshCount; ++subMeshIndex) {
		const SubMeshInfo& subMeshInfo = subMeshInfos[subMeshIndex];
		gpu::SubMeshDrawInfo& info = subMeshDrawInfos[subMeshIndex];
		info._indexCount = subMeshInfo._classicIndexCount;
		info._indexOffset = subMeshInfo._classiciIndexOffset;
	}

	// 読み込み完了
	_meshAssets[meshIndex].closeFile();
	_assetStateFlags[meshIndex] = ASSET_STATE_ENABLE;
}

MeshImpl* MeshResourceManager::allocateMesh(const MeshDesc& desc) {
	u32 meshIndex = _meshes.request();
	MeshImpl* meshImpl = &_meshInstances[meshIndex];

	DebugMeshInfo& debugMeshInfo = _debugMeshInfo[meshIndex];
	memcpy(debugMeshInfo._filePath, desc._filePath, StrLength(desc._filePath));

	u64 fileHash = StrHash(desc._filePath);
	debugMeshInfo._filePathHash = fileHash;
	_fileHashes[meshIndex] = fileHash;

	// アセット実パスに変換
	char meshFilePath[FILE_PATH_COUNT_MAX] = {};
	sprintf_s(meshFilePath, "%s/%s", RESOURCE_FOLDER_PATH, desc._filePath);

	Asset& asset = _meshAssets[meshIndex];
	asset.openFile(meshFilePath);
	asset.setAssetStateFlags(&_assetStateFlags[meshIndex]);
	_assetStateFlags[meshIndex] = ASSET_STATE_ALLOCATED;

	constexpr u32 LOD_PER_MESH_COUNT_MAX = 8;
	constexpr u32 SUBMESH_PER_MESH_COUNT_MAX = 64;
	constexpr u32 MATERIAL_SLOT_COUNT_MAX = 12;
	constexpr u32 MESHLET_INDEX_COUNT = 126;

	// 以下はメッシュエクスポーターと同一の定義にする
	struct SubMeshInfoE {
		u32 _materialSlotIndex = 0;
		u32 _meshletCount = 0;
		u32 _meshletStartIndex = 0;
		u32 _triangleStripIndexCount = 0;
		u32 _triangleStripIndexOffset = 0;
	};

	struct LodInfo {
		u32 _vertexOffset = 0;
		u32 _vertexIndexOffset = 0;
		u32 _primitiveOffset = 0;
		u32 _indexOffset = 0;
		u32 _subMeshOffset = 0;
		u32 _subMeshCount = 0;
		u32 _vertexCount = 0;
		u32 _vertexIndexCount = 0;
		u32 _primitiveCount = 0;
	};
	// =========================================

	// IO読み取り初期化
	FILE* fin = asset.getFile();

	u32 totalLodMeshCount = 0;
	u32 totalSubMeshCount = 0;
	u32 totalMeshletCount = 0;
	u32 totalVertexCount = 0;
	u32 totalVertexIndexCount = 0;
	u32 totalPrimitiveCount = 0;
	u32 materialSlotCount = 0;
	u32 classicIndexCount = 0;
	Float3 boundsMin;
	Float3 boundsMax;
	SubMeshInfoE inputSubMeshInfos[SUBMESH_PER_MESH_COUNT_MAX] = {};
	LodInfo lodInfos[LOD_PER_MESH_COUNT_MAX] = {};
	u64 materialNameHashes[MATERIAL_SLOT_COUNT_MAX] = {};

	// load header
	{
		fread_s(&materialSlotCount, sizeof(u32), sizeof(u32), 1, fin);
		fread_s(&totalSubMeshCount, sizeof(u32), sizeof(u32), 1, fin);
		fread_s(&totalLodMeshCount, sizeof(u32), sizeof(u32), 1, fin);
		fread_s(&totalMeshletCount, sizeof(u32), sizeof(u32), 1, fin);
		fread_s(&totalVertexCount, sizeof(u32), sizeof(u32), 1, fin);
		fread_s(&totalVertexIndexCount, sizeof(u32), sizeof(u32), 1, fin);
		fread_s(&totalPrimitiveCount, sizeof(u32), sizeof(u32), 1, fin);
		fread_s(&boundsMin, sizeof(Float3), sizeof(Float3), 1, fin);
		fread_s(&boundsMax, sizeof(Float3), sizeof(Float3), 1, fin);
		fread_s(&classicIndexCount, sizeof(u32), sizeof(u32), 1, fin);
	}

	VramBufferUpdater* vramUpdater = GraphicsSystemImpl::Get()->getVramUpdater();

	u32 lodMeshStartIndex = _lodMeshes.request(totalLodMeshCount);
	u32 subMeshStartIndex = _subMeshes.request(totalSubMeshCount);
	u32 meshletStartIndex = _meshlets.request(totalMeshletCount);
	LTN_ASSERT(lodMeshStartIndex != MultiDynamicQueueBlockManager::INVILD_BLOCK_INDEX);
	LTN_ASSERT(subMeshStartIndex != MultiDynamicQueueBlockManager::INVILD_BLOCK_INDEX);
	LTN_ASSERT(meshletStartIndex != MultiDynamicQueueBlockManager::INVILD_BLOCK_INDEX);


	{
		fread_s(materialNameHashes, sizeof(u64) * MATERIAL_SLOT_COUNT_MAX, sizeof(u64), materialSlotCount, fin);
		fread_s(inputSubMeshInfos, sizeof(SubMeshInfoE) * SUBMESH_PER_MESH_COUNT_MAX, sizeof(SubMeshInfoE), totalSubMeshCount, fin);
		fread_s(lodInfos, sizeof(LodInfo) * LOD_PER_MESH_COUNT_MAX, sizeof(LodInfo), totalLodMeshCount, fin);
	}

	MeshInfo& meshInfo = _meshInfos[meshIndex];
	meshInfo._meshIndex = meshIndex;
	meshInfo._vertexBinaryIndex = 0;
	meshInfo._indexBinaryIndex = 0;
	meshInfo._classicIndexBinaryIndex = 0;
	meshInfo._vertexCount = totalVertexCount;
	meshInfo._indexCount = totalVertexIndexCount;
	meshInfo._primitiveCount = totalPrimitiveCount;
	meshInfo._totalLodMeshCount = totalLodMeshCount;
	meshInfo._totalSubMeshCount = totalSubMeshCount;
	meshInfo._totalMeshletCount = totalMeshletCount;
	meshInfo._materialSlotCount = materialSlotCount;
	meshInfo._lodMeshStartIndex = lodMeshStartIndex;
	meshInfo._subMeshStartIndex = subMeshStartIndex;
	meshInfo._meshletStartIndex = meshletStartIndex;
	meshInfo._boundsMin = Vector3(boundsMin);
	meshInfo._boundsMax = Vector3(boundsMax);
	meshInfo._classicIndexCount = classicIndexCount;

	for (u32 lodMeshLocalIndex = 0; lodMeshLocalIndex < totalLodMeshCount; ++lodMeshLocalIndex) {
		const LodInfo& info = lodInfos[lodMeshLocalIndex];
		meshInfo._subMeshOffsets[lodMeshLocalIndex] = info._subMeshOffset;
	}

	for (u32 subMeshIndex = 0; subMeshIndex < totalSubMeshCount; ++subMeshIndex) {
		const SubMeshInfoE& inputInfo = inputSubMeshInfos[subMeshIndex];
		SubMeshInfo& info = _subMeshInfos[subMeshStartIndex + subMeshIndex];
		info._meshletOffset = inputInfo._meshletStartIndex;
		info._classicIndexCount = inputInfo._triangleStripIndexCount;
		info._classiciIndexOffset = inputInfo._triangleStripIndexOffset;
		info._meshletCount = inputInfo._meshletCount;
	}

	LTN_ASSERT(materialSlotCount < MeshInfo::MATERIAL_SLOT_COUNT_MAX);
	memcpy(meshInfo._materialSlotNameHashes, materialNameHashes, sizeof(ul64) * materialSlotCount);

	_meshStateFlags[meshIndex] |= MESH_FLAG_STATE_CHANGE;

	// resident メッシュの設定をロードしておく
	gpu::Mesh& mesh = _meshes[meshIndex];
	mesh._stateFlags = gpu::MESH_STATE_ALLOCATED;
	mesh._lodMeshCount = totalLodMeshCount;
	mesh._lodMeshOffset = lodMeshStartIndex;

	for (u32 lodMeshLocalIndex = 0; lodMeshLocalIndex < totalLodMeshCount; ++lodMeshLocalIndex) {
		u32 lodMeshIndex = lodMeshStartIndex + lodMeshLocalIndex;
		const LodInfo& info = lodInfos[lodMeshLocalIndex];
		gpu::LodMesh& lodMesh = _lodMeshes[lodMeshIndex];
		lodMesh._subMeshOffset = subMeshStartIndex + info._subMeshOffset;
		lodMesh._subMeshCount = info._subMeshCount;
		lodMesh._vertexOffset = info._vertexOffset;
		lodMesh._vertexIndexOffset = info._vertexIndexOffset;
		lodMesh._primitiveOffset = info._primitiveOffset;
	}

	for (u32 subMeshLocalIndex = 0; subMeshLocalIndex < totalSubMeshCount; ++subMeshLocalIndex) {
		u32 subMeshIndex = subMeshStartIndex + subMeshLocalIndex;
		const SubMeshInfoE& info = inputSubMeshInfos[subMeshLocalIndex];
		gpu::SubMesh& subMesh = _subMeshes[subMeshIndex];
		subMesh._meshletOffset = info._meshletStartIndex + meshletStartIndex;
		subMesh._meshletCount = info._meshletCount;
		subMesh._materialSlotIndex = info._materialSlotIndex;
	}

	gpu::Mesh* meshes = vramUpdater->enqueueUpdate<gpu::Mesh>(&_meshBuffer, sizeof(gpu::Mesh) * meshIndex);
	memcpy(meshes, &_meshes[meshIndex], sizeof(gpu::Mesh));

	asset.updateCurrentSeekPosition();
	asset.closeFile();

	meshImpl->setAsset(&_meshAssets[meshIndex]);
	meshImpl->setMesh(&_meshes[meshIndex]);
	meshImpl->setLodMeshes(&_lodMeshes[lodMeshStartIndex]);
	meshImpl->setSubMeshes(&_subMeshes[subMeshStartIndex]);
	meshImpl->setMeshlets(&_meshlets[meshletStartIndex]);
	meshImpl->setMeshInfo(&_meshInfos[meshIndex]);
	meshImpl->setSubMeshInfos(&_subMeshInfos[subMeshStartIndex]);
	meshImpl->setStateFlags(&_meshStateFlags[meshIndex]);
	meshImpl->setDebugMeshInfo(&_debugMeshInfo[meshIndex]);

	return meshImpl;
}

Mesh* MeshResourceManager::createMesh(const MeshDesc& desc) {
	u64 fileHash = StrHash(desc._filePath);
	u32 searchedMeshIndex = getMeshIndexFromFileHash(fileHash);
	if (searchedMeshIndex != gpu::INVALID_INDEX) {
		return &_meshInstances[searchedMeshIndex];
	}

	MeshImpl* mesh = allocateMesh(desc);
	mesh->getAsset()->requestLoad();
	return mesh;
}

MeshImpl* MeshResourceManager::findMesh(u64 fileHash) {
	u32 meshIndex = getMeshIndexFromFileHash(fileHash);
	LTN_ASSERT(meshIndex != gpu::INVALID_INDEX);
	return &_meshInstances[meshIndex];
}

u32 MeshResourceManager::getMeshIndexFromFileHash(ul64 fileHash) const {
	u32 findMeshIndex = gpu::INVALID_INDEX;
	u32 meshCount = _meshes.getResarveCount();
	for (u32 meshIndex = 0; meshIndex < meshCount; ++meshIndex) {
		if (_fileHashes[meshIndex] == fileHash) {
			findMeshIndex = meshIndex;
			break;
		}
	}

	return findMeshIndex;
}

u32 MeshResourceManager::getMeshIndex(const MeshInfo* meshInfo) const {
	u32 index = static_cast<u32>(meshInfo - _meshInfos);
	LTN_ASSERT(index < MESH_COUNT_MAX);
	return index;
}

GpuDescriptorHandle MeshResourceManager::getSubMeshSrv() const {
	u64 incrementSize = GraphicsSystemImpl::Get()->getSrvCbvUavGpuDescriptorAllocator()->getIncrimentSize();
	return _meshSrv._gpuHandle + incrementSize * 2;
}

MeshResourceManagerInfo MeshResourceManager::getMeshResourceInfo() const {
	MeshResourceManagerInfo info;
	info._meshCount = _meshes.getInstanceCount();
	info._lodMeshCount = _lodMeshes.getInstanceCount();
	info._subMeshCount = _subMeshes.getInstanceCount();
	info._meshletCount = _meshlets.getInstanceCount();
	info._vertexCount = _vertexPositionBinaryHeaders.getInstanceCount();
	info._triangleCount = _primitiveBinaryHeaders.getInstanceCount();
	return info;
}

void MeshResourceManager::deleteMesh(u32 meshIndex) {
	const MeshInfo& meshInfo = _meshInfos[meshIndex];
	_vertexPositionBinaryHeaders.discard(meshInfo._vertexBinaryIndex, meshInfo._vertexCount);
	_meshlets.discard(&_meshlets[meshInfo._meshletStartIndex], meshInfo._totalMeshletCount);
	_subMeshes.discard(&_subMeshes[meshInfo._subMeshStartIndex], meshInfo._totalSubMeshCount);
	_lodMeshes.discard(&_lodMeshes[meshInfo._lodMeshStartIndex], meshInfo._totalLodMeshCount);
	_meshes.discard(meshIndex);

	for (u32 subMeshIndex = 0; subMeshIndex < meshInfo._totalSubMeshCount; ++subMeshIndex) {
		_subMeshInfos[meshInfo._subMeshStartIndex + subMeshIndex] = SubMeshInfo();
	}
	_assetStateFlags[meshIndex] = ASSET_STATE_NONE;
	_meshStateFlags[meshIndex] = MESH_FLAG_STATE_NONE;
	_fileHashes[meshIndex] = 0;
	_meshInfos[meshIndex] = MeshInfo();
	_meshInstances[meshIndex] = MeshImpl();
	_debugMeshInfo[meshIndex] = DebugMeshInfo();
}

void Mesh::requestToDelete(){
	*_stateFlags |= MESH_FLAG_STATE_REQUEST_DESTROY;
}