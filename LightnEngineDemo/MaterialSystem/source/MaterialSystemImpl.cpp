#include <MaterialSystem/impl/MaterialSystemImpl.h>
#include <GfxCore/impl/GraphicsSystemImpl.h>
#include <GfxCore/impl/GraphicsApiInterface.h>
#include <MeshRenderer/GpuStruct.h>
#include <TextureSystem/impl/TextureSystemImpl.h>
#include <fstream>

MaterialSystemImpl _materialSystem;

void MaterialSystemImpl::initialize() {
	_materials.initialize(MATERIAL_COUNT_MAX);
	_shaderSets.initialize(SHADER_SET_COUNT_MAX);
	PipelineStateSystem::Get()->initialize();
}

void MaterialSystemImpl::update() {
	PipelineStateSystem::Get()->update();
}

void MaterialSystemImpl::processDeletion() {
	u32 materialCount = _materials.getResarveCount();
	for (u32 materialIndex = 0; materialIndex < materialCount; ++materialIndex) {
		if (_materialStateFlags[materialIndex] & MATERIAL_STATE_FLAG_CREATED) {
			_materialStateFlags[materialIndex] = MATERIAL_STATE_FLAG_ENABLED;
		}

		if (_materialStateFlags[materialIndex] & MATERIAL_STATE_FLAG_REQEST_DESTROY) {
			MaterialImpl& material = _materials[materialIndex];
			ShaderSetImpl* shaderSet = material.getShaderSet();
			u32 shaderParamIndex = static_cast<u32>(material.getShaderSetStateFlags() - shaderSet->_shaderParamStateFlags);
			shaderSet->_shaderParamInstances.discard(shaderParamIndex);

			_materialStateFlags[materialIndex] = MATERIAL_STATE_FLAG_NONE;
			_materialFileHashes[materialIndex] = 0;
			_materials.discard(materialIndex);
		}
	}

	for (u32 materialIndex = 0; materialIndex < materialCount; ++materialIndex) {
		if (_materialUpdateFlags[materialIndex] & MATERIAL_UPDATE_FLAG_UPDATE_PARAMS) {
			_materialUpdateFlags[materialIndex] &= ~MATERIAL_UPDATE_FLAG_UPDATE_PARAMS;
		}
	}

	u32 shaderSetCount = _shaderSets.getResarveCount();
	for (u32 shaderSetIndex = 0; shaderSetIndex < shaderSetCount; ++shaderSetIndex) {
		if (_shaderSetStateFlags[shaderSetIndex] & SHADER_SET_STATE_FLAG_CREATED) {
			_shaderSetStateFlags[shaderSetIndex] = SHADER_SET_STATE_FLAG_ENABLED;
		}

		if (_shaderSetStateFlags[shaderSetIndex] & SHADER_SET_STATE_FLAG_REQEST_DESTROY) {
			_shaderSetStateFlags[shaderSetIndex] = SHADER_SET_STATE_FLAG_NONE;
			_shaderSets[shaderSetIndex].terminate();
			_pipelineStateSets[TYPE_AS_MESH_SHADER].requestDelete(shaderSetIndex);
			_pipelineStateSets[TYPE_MESH_SHADER].requestDelete(shaderSetIndex);
			_pipelineStateSets[TYPE_CLASSIC].requestDelete(shaderSetIndex);

			_shaderSetFileHashes[shaderSetIndex] = 0;
			_shaderSets[shaderSetIndex] = ShaderSetImpl();

			_shaderSets.discard(shaderSetIndex);
		}
	}

	PipelineStateSystem::Get()->processDeletion();
}

void MaterialSystemImpl::terminate() {
	processDeletion();
	LTN_ASSERT(_materials.getInstanceCount() == 0);
	_materials.terminate();
	_shaderSets.terminate();
	PipelineStateSystem::Get()->terminate();
}

u32 MaterialSystemImpl::findShaderSetIndex(u64 fileHash) {
	u32 pipelineStateCount = _shaderSets.getResarveCount();
	for (u32 shaderSetIndex = 0; shaderSetIndex < pipelineStateCount; ++shaderSetIndex) {
		if (_shaderSetFileHashes[shaderSetIndex] == fileHash) {
			return shaderSetIndex;
		}
	}
	return gpu::INVALID_INDEX;
}

u32 MaterialSystemImpl::getShaderSetIndex(const ShaderSetImpl* shaderSet) const {
	return _shaderSets.getArrayIndex(shaderSet);
}

u32 MaterialSystemImpl::getMaterialIndex(const Material* material) const {
	return _materials.getArrayIndex(static_cast<const MaterialImpl*>(material));
}

bool MaterialSystemImpl::isEnabledShaderSet(const ShaderSetImpl* shaderSet) const {
	return !(_shaderSetStateFlags[getShaderSetIndex(shaderSet)] & SHADER_SET_STATE_FLAG_NONE);
}

ShaderSet* MaterialSystemImpl::createShaderSet(const ShaderSetDesc& desc) {
	u64 fileHash = StrHash(desc._filePath);
	u32 findIndex = findShaderSetIndex(fileHash);

	if (findIndex == gpu::INVALID_INDEX) {
		findIndex = _shaderSets.request();

		PipelineStateSet& meshShaderPipelineStateSet = _pipelineStateSets[TYPE_AS_MESH_SHADER];
		PipelineStateSet& primInstancingPipelineStateSet = _pipelineStateSets[TYPE_MESH_SHADER];
		PipelineStateSet& classicPipelineStateSet = _pipelineStateSets[TYPE_CLASSIC];

		ShaderSetImplDesc implDesc = {};
		implDesc._debugCullingPassPipelineStateGroup = &meshShaderPipelineStateSet._debugCullingPassPipelineStateGroups[findIndex];
		implDesc._debugDepthPipelineStateGroup = &meshShaderPipelineStateSet._debugDepthPipelineStateGroups[findIndex];
		implDesc._debugLodLevelPipelineStateGroup = &meshShaderPipelineStateSet._debugLodLevelPipelineStateGroups[findIndex];
		implDesc._debugMeshletPipelineStateGroup = &meshShaderPipelineStateSet._debugMeshletPipelineStateGroups[findIndex];
		implDesc._debugOcclusionPipelineStateGroup = &meshShaderPipelineStateSet._debugOcclusionPipelineStateGroups[findIndex];
		implDesc._debugTexcoordsPipelineStateGroup = &meshShaderPipelineStateSet._debugTexcoordsPipelineStateGroups[findIndex];
		implDesc._debugWireFramePipelineStateGroup = &meshShaderPipelineStateSet._debugWireFramePipelineStateGroups[findIndex];
		implDesc._commandSignature = &meshShaderPipelineStateSet._commandSignatures[findIndex];

		implDesc._debugPrimCullingPassPipelineStateGroup = &primInstancingPipelineStateSet._debugCullingPassPipelineStateGroups[findIndex];
		implDesc._debugPrimDepthPipelineStateGroup = &primInstancingPipelineStateSet._debugDepthPipelineStateGroups[findIndex];
		implDesc._debugPrimLodLevelPipelineStateGroup = &primInstancingPipelineStateSet._debugLodLevelPipelineStateGroups[findIndex];
		implDesc._debugPrimMeshletPipelineStateGroup = &primInstancingPipelineStateSet._debugMeshletPipelineStateGroups[findIndex];
		implDesc._debugPrimOcclusionPipelineStateGroup = &primInstancingPipelineStateSet._debugOcclusionPipelineStateGroups[findIndex];
		implDesc._debugPrimTexcoordsPipelineStateGroup = &primInstancingPipelineStateSet._debugTexcoordsPipelineStateGroups[findIndex];
		implDesc._debugPrimWireFramePipelineStateGroup = &primInstancingPipelineStateSet._debugWireFramePipelineStateGroups[findIndex];
		implDesc._msCommandSignature = &primInstancingPipelineStateSet._commandSignatures[findIndex];

		implDesc._depthPipelineStateGroup = &meshShaderPipelineStateSet._depthPipelineStateGroups[findIndex];
		implDesc._pipelineStateGroup = &meshShaderPipelineStateSet._pipelineStateGroups[findIndex];
		implDesc._primPipelineStateGroup = &primInstancingPipelineStateSet._pipelineStateGroups[findIndex];
		implDesc._primDepthPipelineStateGroup = &primInstancingPipelineStateSet._depthPipelineStateGroups[findIndex];
		implDesc._classicPipelineStateGroup = &classicPipelineStateSet._pipelineStateGroups[findIndex];
		implDesc._classicDepthPipelineStateGroup = &classicPipelineStateSet._depthPipelineStateGroups[findIndex];
		implDesc._multiDrawCommandSignature = &classicPipelineStateSet._commandSignatures[findIndex];

		ShaderSetImpl& shaderSet = _shaderSets[findIndex];
		shaderSet.initialize(desc, implDesc);
		shaderSet.setStateFlags(&_shaderSetStateFlags[findIndex]);
		_shaderSetFileHashes[findIndex] = fileHash;
		_shaderSetStateFlags[findIndex] |= SHADER_SET_STATE_FLAG_CREATED;
	}

	ShaderSetImpl* shaderSet = &_shaderSets[findIndex];
	return shaderSet;
}

Material* MaterialSystemImpl::createMaterial(const MaterialDesc& desc) {
	// アセット実パスに変換
	char meshFilePath[FILE_PATH_COUNT_MAX] = {};
	sprintf_s(meshFilePath, "%s/%s", RESOURCE_FOLDER_PATH, desc._filePath);

	// IO読み取り初期化
	std::ifstream fin(meshFilePath, std::ios::in | std::ios::binary);
	fin.exceptions(std::ios::badbit);
	LTN_ASSERT(!fin.fail());

	u64 shaderSetFileHash = 0;
	fin.read(reinterpret_cast<char*>(&shaderSetFileHash), sizeof(u64));

	u32 textureCount = 0;
	u64 textureFileHashes[32] = {};
	fin.read(reinterpret_cast<char*>(&textureCount), sizeof(u32));
	for (u32 textureIndex = 0; textureIndex < textureCount; ++textureIndex) {
		fin.read(reinterpret_cast<char*>(&textureFileHashes[textureIndex]), sizeof(u64));
	}

	fin.close();

	u32 shaderSetIndex = findShaderSetIndex(shaderSetFileHash);
	LTN_ASSERT(shaderSetIndex != static_cast<u32>(-1));

	ShaderSetImpl* shaderSet = &_shaderSets[shaderSetIndex];
	u32 shaderSetMaterialIndex = shaderSet->_shaderParamInstances.request();

	u32 materialIndex = _materials.request();
	MaterialImpl* material = &_materials[materialIndex];
	material->setShaderSetStateFlags(&shaderSet->_shaderParamStateFlags[shaderSetMaterialIndex]);
	material->setParameterDataPtr(&shaderSet->_parameterDatas[shaderSet->_parameterSizeInByte*shaderSetMaterialIndex]);
	material->setShaderSet(shaderSet);
	material->setStateFlags(&_materialStateFlags[materialIndex]);
	material->setUpdateFlags(&_materialUpdateFlags[materialIndex]);
	_materialFileHashes[materialIndex] = StrHash(desc._filePath);
	_materialStateFlags[materialIndex] |= MATERIAL_STATE_FLAG_CREATED;
	_materialUpdateFlags[materialIndex] |= MATERIAL_UPDATE_FLAG_UPDATE_PARAMS;

	material->setParameter<Color4>(StrHash32("BaseColor"), Color4::WHITE);
	material->setTexture(StrHash32("AlbedoTextureIndex"), TextureSystemImpl::Get()->findTexture(textureFileHashes[0]));

	return material;
}

Material* MaterialSystemImpl::findMaterial(u64 filePathHash) {
	u32 materialCount = _materials.getResarveCount();
	for (u32 materialIndex = 0; materialIndex < materialCount; ++materialIndex) {
		if (_materialFileHashes[materialIndex] == filePathHash) {
			return &_materials[materialIndex];
		}
	}

	return nullptr;
}

MaterialSystemImpl* MaterialSystemImpl::Get() {
	return &_materialSystem;
}

MaterialSystem* MaterialSystem::Get() {
	return &_materialSystem;
}

void MaterialImpl::requestToDelete() {
	_shaderSet->requestToDelete();
	*_stateFlags |= MATERIAL_STATE_FLAG_REQEST_DESTROY;
}

void MaterialImpl::setTexture(u32 nameHash, Texture* texture) {
	setParameter<u32>(nameHash, TextureSystemImpl::Get()->getTextureIndex(texture));
}

const u8* MaterialImpl::getParameterRaw(u32 nameHash) const {
	u16 findIndex = findParameter(nameHash);
	if (findIndex == INVALID_PARAMETER_INDEX) {
		return nullptr;
	}

	return &_parameterData[_shaderSet->_parameterByteOffset[findIndex]];
}

void MaterialImpl::setParameterRaw(u32 nameHash, const void* dataPtr) {
	u16 findIndex = findParameter(nameHash);
	if (findIndex == INVALID_PARAMETER_INDEX) {
		return;
	}

	u16 copySizeInByte = PARAM_TYPE_SIZE_IN_BYTE[_shaderSet->_parameterTypes[findIndex]];
	u16 offset = _shaderSet->_parameterByteOffset[findIndex];
	u8* basePtr = &_parameterData[offset];
	memcpy(basePtr, dataPtr, copySizeInByte);

	*_updateFlags |= MATERIAL_UPDATE_FLAG_UPDATE_PARAMS;
}

u16 MaterialImpl::findParameter(u32 nameHash) const {
	u16 parameterCount = _shaderSet->_parameterCount;
	u32* parameterNameHashes = _shaderSet->_parameterNameHashes;
	for (u16 parameterIndex = 0; parameterIndex < parameterCount; ++parameterIndex) {
		if (nameHash == parameterNameHashes[parameterIndex]) {
			return parameterIndex;
		}
	}

	return INVALID_PARAMETER_INDEX;
}

void ShaderSetImpl::requestToDelete() {
	*_stateFlags |= SHADER_SET_STATE_FLAG_REQEST_DESTROY;
}

void ShaderSetImpl::setTexture(Texture* texture, u64 parameterNameHash) {
}

void ShaderSetImpl::initialize(const ShaderSetDesc& desc, ShaderSetImplDesc& implDesc) {
	_shaderParamInstances.initialize(MATERIAL_COUNT_MAX);

	u32 meshShaderNameLength = 0;
	u32 pixelShaderNameLength = 0;
	char meshShaderName[FILE_PATH_COUNT_MAX] = {};
	char pixelShaderName[FILE_PATH_COUNT_MAX] = {};

	// シェーダーパス
	{
		char shaderSetFilePath[FILE_PATH_COUNT_MAX] = {};
		sprintf_s(shaderSetFilePath, "%s/%s", RESOURCE_FOLDER_PATH, desc._filePath);

		std::ifstream fin(shaderSetFilePath, std::ios::in | std::ios::binary);
		fin.exceptions(std::ios::badbit);
		LTN_ASSERT(!fin.fail());

		fin.read(reinterpret_cast<char*>(&meshShaderNameLength), sizeof(u32));
		fin.read(reinterpret_cast<char*>(meshShaderName), meshShaderNameLength);
		fin.read(reinterpret_cast<char*>(&pixelShaderNameLength), sizeof(u32));
		fin.read(reinterpret_cast<char*>(pixelShaderName), pixelShaderNameLength);

		fin.close();
	}

	char meshShaderPath[FILE_PATH_COUNT_MAX] = {};
	char pixelShaderPath[FILE_PATH_COUNT_MAX] = {};
	sprintf_s(meshShaderPath, "%s/%s", RESOURCE_FOLDER_PATH, meshShaderName);
	sprintf_s(pixelShaderPath, "%s/%s", RESOURCE_FOLDER_PATH, pixelShaderName);

	// シェーダーパラメーター
	{
		char shaderSetFilePath[FILE_PATH_COUNT_MAX] = {};
		sprintf_s(shaderSetFilePath, "%s/%s", RESOURCE_FOLDER_PATH, pixelShaderName);

		// シェーダーパスから Info パスに変換
		char* str = shaderSetFilePath;
		while (*str != '.') {
			str++;
		}

		// .vso -> .vsInfo
		// .pso -> .psinfo
		memcpy(str + 3, "info\0", 6);

		std::ifstream fin(shaderSetFilePath, std::ios::in | std::ios::binary);
		fin.exceptions(std::ios::badbit);
		LTN_ASSERT(!fin.fail());

		u16 parameterSizeInByte = 0;
		u16 parameterCount = 0;
		fin.read(reinterpret_cast<char*>(&parameterCount), sizeof(u16));
		for (u32 parameterIndex = 0; parameterIndex < parameterCount; ++parameterIndex) {
			u32& nameHash = _parameterNameHashes[parameterIndex];
			u16& type = _parameterTypes[parameterIndex];
			fin.read(reinterpret_cast<char*>(&nameHash), sizeof(u32));
			fin.read(reinterpret_cast<char*>(&type), sizeof(u16));
			_parameterByteOffset[parameterIndex] = parameterSizeInByte;
			parameterSizeInByte += Material::PARAM_TYPE_SIZE_IN_BYTE[type];
		}

		_parameterSizeInByte = parameterSizeInByte;
		_parameterCount = parameterCount;

		fin.close();
	}

	_parameterDatas = new u8[_parameterSizeInByte * MATERIAL_COUNT_MAX];

	constexpr u32 TEXTURE_BASE_REGISTER = 29;
	Device* device = GraphicsSystemImpl::Get()->getDevice();
	DescriptorRange cullingViewCbvRange(DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	DescriptorRange viewCbvRange(DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	DescriptorRange materialDescriptorRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	DescriptorRange meshDescriptorRange(DESCRIPTOR_RANGE_TYPE_SRV, 5, 1);
	DescriptorRange meshInstanceDescriptorRange(DESCRIPTOR_RANGE_TYPE_SRV, 4, 6);
	DescriptorRange meshletInfoSrvRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 10);
	DescriptorRange vertexDescriptorRange(DESCRIPTOR_RANGE_TYPE_SRV, 5, 11);
	DescriptorRange currentLodLevelRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 16);
	DescriptorRange meshletInstancePrimitiveInfoRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 17);
	DescriptorRange meshletInstanceMeshInstanceIndexRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 18);
	DescriptorRange meshletInstanceWorldMatrixRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 19);
	DescriptorRange hizRange(DESCRIPTOR_RANGE_TYPE_SRV, 8, 20);
	DescriptorRange textureDescriptorRange(DESCRIPTOR_RANGE_TYPE_SRV, 128, TEXTURE_BASE_REGISTER);
	DescriptorRange cullingResultDescriptorRange(DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	constexpr u32 ROOT_FRUSTUM_COUNT = DefaultMeshRootParam::ROOT_DEFAULT_MESH_COUNT - 2;
	RootParameter furstumCullingRootParameters[ROOT_FRUSTUM_COUNT] = {};
	RootParameter furstumOcclusionCullingRootParameters[DefaultMeshRootParam::ROOT_DEFAULT_MESH_COUNT] = {};

	RootSignatureDesc rootSignatureDescFurstumCulling = {};
	RootSignatureDesc rootSignatureDescFurstumOcclusionCulling = {};

	// メッシュレット　フラスタムカリングのみ
	{
		RootParameter* rootParameters = furstumCullingRootParameters;
		rootParameters[DefaultMeshRootParam::CULLING_VIEW_CONSTANT].initializeDescriptorTable(1, &cullingViewCbvRange, SHADER_VISIBILITY_AMPLIFICATION);
		rootParameters[DefaultMeshRootParam::VIEW_CONSTANT].initializeDescriptorTable(1, &viewCbvRange, SHADER_VISIBILITY_ALL);
		rootParameters[DefaultMeshRootParam::MATERIALS].initializeDescriptorTable(1, &materialDescriptorRange, SHADER_VISIBILITY_ALL);
		rootParameters[DefaultMeshRootParam::MESH].initializeDescriptorTable(1, &meshDescriptorRange, SHADER_VISIBILITY_ALL);
		rootParameters[DefaultMeshRootParam::MESH_INSTANCE].initializeDescriptorTable(1, &meshInstanceDescriptorRange, SHADER_VISIBILITY_ALL);
		rootParameters[DefaultMeshRootParam::INDIRECT_CONSTANT].initializeConstant(2, 4, SHADER_VISIBILITY_ALL);
		rootParameters[DefaultMeshRootParam::MESHLET_INFO].initializeDescriptorTable(1, &meshletInfoSrvRange, SHADER_VISIBILITY_ALL);
		rootParameters[DefaultMeshRootParam::VERTEX_RESOURCES].initializeDescriptorTable(1, &vertexDescriptorRange, SHADER_VISIBILITY_MESH);
		rootParameters[DefaultMeshRootParam::TEXTURES].initializeDescriptorTable(1, &textureDescriptorRange, SHADER_VISIBILITY_ALL);
		rootParameters[DefaultMeshRootParam::LOD_LEVEL].initializeDescriptorTable(1, &currentLodLevelRange, SHADER_VISIBILITY_ALL);
		rootParameters[DefaultMeshRootParam::MESHLET_PRIMITIVE_INFO].initializeDescriptorTable(1, &meshletInstancePrimitiveInfoRange, SHADER_VISIBILITY_ALL);
		rootParameters[DefaultMeshRootParam::MESHLET_MESH_INSTANCE_INDEX].initializeDescriptorTable(1, &meshletInstanceMeshInstanceIndexRange, SHADER_VISIBILITY_ALL);
		rootParameters[DefaultMeshRootParam::MESH_INSTANCE_WORLD_MATRIX].initializeDescriptorTable(1, &meshletInstanceWorldMatrixRange, SHADER_VISIBILITY_ALL);

		rootSignatureDescFurstumCulling._device = device;
		rootSignatureDescFurstumCulling._numParameters = LTN_COUNTOF(furstumCullingRootParameters);
		rootSignatureDescFurstumCulling._parameters = rootParameters;
		memcpy(furstumOcclusionCullingRootParameters, rootParameters, sizeof(RootParameter)*ROOT_FRUSTUM_COUNT);
	}

	// メッシュレット　フラスタム + オクルージョンカリング
	{
		RootParameter* rootParameters = furstumOcclusionCullingRootParameters;
		rootParameters[DefaultMeshRootParam::CULLING_RESULT].initializeDescriptorTable(1, &cullingResultDescriptorRange, SHADER_VISIBILITY_ALL);
		rootParameters[DefaultMeshRootParam::HIZ].initializeDescriptorTable(1, &hizRange, SHADER_VISIBILITY_ALL);

		rootSignatureDescFurstumOcclusionCulling._device = device;
		rootSignatureDescFurstumOcclusionCulling._numParameters = LTN_COUNTOF(furstumOcclusionCullingRootParameters);
		rootSignatureDescFurstumOcclusionCulling._parameters = rootParameters; 
	}

	PipelineStateSystem* pipelineStateSystem = PipelineStateSystem::Get();
	RenderTargetBlendDesc debugOcclusionBlendDesc = {};
	debugOcclusionBlendDesc._blendEnable = true;
	debugOcclusionBlendDesc._srcBlend = BLEND_SRC_ALPHA;
	debugOcclusionBlendDesc._destBlend = BLEND_INV_SRC_ALPHA;
	debugOcclusionBlendDesc._blendOp = BLEND_OP_ADD;
	debugOcclusionBlendDesc._srcBlendAlpha = BLEND_ONE;
	debugOcclusionBlendDesc._destBlendAlpha = BLEND_ZERO;
	debugOcclusionBlendDesc._blendOpAlpha = BLEND_OP_ADD;

	constexpr char msPrimitiveInstancingFilePath[] = "L:\\LightnEngine\\resource\\common\\shader\\standard_mesh\\default_mesh_primitive_instancing.mso";
	constexpr char asMeshletCullingFilePath[] = "L:\\LightnEngine\\resource\\common\\shader\\meshlet\\meshlet_culling_pass.aso";
	constexpr char asMeshletCullingFrustumOcclusionFilePath[] = "L:\\LightnEngine\\resource\\common\\shader\\meshlet\\meshlet_culling_frustum_occlusion.aso";
	constexpr char asMeshletCullingFrustumFilePath[] = "L:\\LightnEngine\\resource\\common\\shader\\meshlet\\meshlet_culling_frustum.aso";
	constexpr char psDebugMeshletFilePath[] = "L:\\LightnEngine\\resource\\common\\shader\\debug\\debug_meshlet.pso";
	constexpr char psDebugLodFilePath[] = "L:\\LightnEngine\\resource\\common\\shader\\debug\\debug_lod.pso";
	constexpr char psDebugDepthFilePath[] = "L:\\LightnEngine\\resource\\common\\shader\\debug\\debug_depth.pso";
	constexpr char psDebugTexCoordFilePath[] = "L:\\LightnEngine\\resource\\common\\shader\\debug\\debug_texcoords.pso";
	constexpr char psDebugWireFilePath[] = "L:\\LightnEngine\\resource\\common\\shader\\debug\\debug_wireframe.pso";
	constexpr char psDebugOcclusionFilePath[] = "L:\\LightnEngine\\resource\\common\\shader\\debug\\debug_occlusion_culling.pso";

	// メッシュシェーダーのみ
	{
		MeshShaderPipelineStateGroupDesc pipelineStateDesc = {};
		pipelineStateDesc._depthComparisonFunc = COMPARISON_FUNC_LESS_EQUAL;

		RootSignatureDesc rootSignatureDesc = rootSignatureDescFurstumCulling;
		rootSignatureDesc._parameters = furstumCullingRootParameters;

		// Depth Only
		pipelineStateDesc._meshShaderFilePath = msPrimitiveInstancingFilePath;
		*implDesc._primDepthPipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(pipelineStateDesc, rootSignatureDesc);

		// フラスタム ＋ オクルージョンカリング
		pipelineStateDesc._pixelShaderFilePath = pixelShaderPath;
		*implDesc._primPipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(pipelineStateDesc, rootSignatureDesc);

		// メッシュレットデバッグ用
		pipelineStateDesc._pixelShaderFilePath = psDebugMeshletFilePath;
		*implDesc._debugPrimMeshletPipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(pipelineStateDesc, rootSignatureDesc);

		// LodLevel デバッグ用
		pipelineStateDesc._pixelShaderFilePath = psDebugLodFilePath;
		*implDesc._debugPrimLodLevelPipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(pipelineStateDesc, rootSignatureDesc);

		// Depth デバッグ用
		pipelineStateDesc._pixelShaderFilePath = psDebugDepthFilePath;
		*implDesc._debugPrimDepthPipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(pipelineStateDesc, rootSignatureDesc);

		// TexCoords デバッグ用
		pipelineStateDesc._pixelShaderFilePath = psDebugTexCoordFilePath;
		*implDesc._debugPrimTexcoordsPipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(pipelineStateDesc, rootSignatureDesc);

		// ワイヤーフレーム表示
		{
			MeshShaderPipelineStateGroupDesc desc = pipelineStateDesc;
			desc._fillMode = FILL_MODE_WIREFRAME;
			desc._pixelShaderFilePath = psDebugWireFilePath;
			*implDesc._debugPrimWireFramePipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(desc, rootSignatureDesc);
		}

		// オクルージョンカリング可視化
		pipelineStateDesc._pixelShaderFilePath = psDebugOcclusionFilePath;
		pipelineStateDesc._depthComparisonFunc = COMPARISON_FUNC_ALWAYS;
		pipelineStateDesc._blendDesc._renderTarget[0] = debugOcclusionBlendDesc;
		*implDesc._debugPrimOcclusionPipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(pipelineStateDesc, rootSignatureDesc);
	}

	// メッシュシェーダー + 増幅シェーダー
	{
		MeshShaderPipelineStateGroupDesc pipelineStateDesc = {};
		pipelineStateDesc._depthComparisonFunc = COMPARISON_FUNC_LESS_EQUAL;
		pipelineStateDesc._meshShaderFilePath = meshShaderPath;

		// GPU カリング無効デバッグ用
		pipelineStateDesc._amplificationShaderFilePath = asMeshletCullingFilePath;
		pipelineStateDesc._pixelShaderFilePath = pixelShaderPath;
		*implDesc._debugCullingPassPipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(pipelineStateDesc, rootSignatureDescFurstumOcclusionCulling);

		// フラスタム ＋ オクルージョンカリング
		pipelineStateDesc._amplificationShaderFilePath = asMeshletCullingFrustumOcclusionFilePath;
		*implDesc._pipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(pipelineStateDesc, rootSignatureDescFurstumOcclusionCulling);

		// メッシュレットデバッグ用
		pipelineStateDesc._pixelShaderFilePath = psDebugMeshletFilePath;
		*implDesc._debugMeshletPipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(pipelineStateDesc, rootSignatureDescFurstumOcclusionCulling);

		// LodLevel デバッグ用
		pipelineStateDesc._pixelShaderFilePath = psDebugLodFilePath;
		*implDesc._debugLodLevelPipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(pipelineStateDesc, rootSignatureDescFurstumOcclusionCulling);

		// Depth デバッグ用
		pipelineStateDesc._pixelShaderFilePath = psDebugDepthFilePath;
		*implDesc._debugDepthPipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(pipelineStateDesc, rootSignatureDescFurstumOcclusionCulling);

		// TexCoords デバッグ用
		pipelineStateDesc._pixelShaderFilePath = psDebugTexCoordFilePath;
		*implDesc._debugTexcoordsPipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(pipelineStateDesc, rootSignatureDescFurstumOcclusionCulling);

		// ワイヤーフレーム表示
		{
			MeshShaderPipelineStateGroupDesc desc = pipelineStateDesc;
			desc._fillMode = FILL_MODE_WIREFRAME;
			desc._pixelShaderFilePath = psDebugWireFilePath;
			*implDesc._debugWireFramePipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(desc, rootSignatureDescFurstumOcclusionCulling);
		}

		// デプスオンリー (フラスタムカリングのみ)
		pipelineStateDesc._amplificationShaderFilePath = asMeshletCullingFrustumFilePath;
		pipelineStateDesc._pixelShaderFilePath = nullptr;
		*implDesc._depthPipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(pipelineStateDesc, rootSignatureDescFurstumOcclusionCulling);

		// オクルージョンカリング可視化
		pipelineStateDesc._pixelShaderFilePath = psDebugOcclusionFilePath;
		pipelineStateDesc._depthComparisonFunc = COMPARISON_FUNC_ALWAYS;
		pipelineStateDesc._blendDesc._renderTarget[0] = debugOcclusionBlendDesc;
		*implDesc._debugOcclusionPipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(pipelineStateDesc, rootSignatureDescFurstumOcclusionCulling);
	}

	// classic
	{
		DescriptorRange cbvDescriptorRange(DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		DescriptorRange meshInstanceDescriptorRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		DescriptorRange materialDescriptorRange(DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		DescriptorRange textureDescriptorRange(DESCRIPTOR_RANGE_TYPE_SRV, 128, TEXTURE_BASE_REGISTER);

		RootParameter rootParameters[ClassicMeshRootParam::COUNT] = {};
		rootParameters[ClassicMeshRootParam::SCENE_CONSTANT].initializeDescriptorTable(1, &cbvDescriptorRange, SHADER_VISIBILITY_ALL);
		rootParameters[ClassicMeshRootParam::MESH_INFO].initializeConstant(1, 2, SHADER_VISIBILITY_VERTEX);
		rootParameters[ClassicMeshRootParam::MATERIALS].initializeDescriptorTable(1, &materialDescriptorRange, SHADER_VISIBILITY_PIXEL);
		rootParameters[ClassicMeshRootParam::MESH_INSTANCE].initializeDescriptorTable(1, &meshInstanceDescriptorRange, SHADER_VISIBILITY_VERTEX);
		rootParameters[ClassicMeshRootParam::TEXTURES].initializeDescriptorTable(1, &textureDescriptorRange, SHADER_VISIBILITY_PIXEL);

		RootSignatureDesc rootSignatureDesc = {};
		rootSignatureDesc._device = device;
		rootSignatureDesc._numParameters = LTN_COUNTOF(rootParameters);
		rootSignatureDesc._parameters = rootParameters;
		rootSignatureDesc._flags = ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		char vertexShaderName[FILE_PATH_COUNT_MAX] = {};
		memcpy(vertexShaderName, meshShaderName, meshShaderNameLength - 3);
		memcpy(vertexShaderName + meshShaderNameLength - 3, "vso", 3);

		char vertexShaderPath[FILE_PATH_COUNT_MAX] = {};
		sprintf_s(vertexShaderPath, "%s/%s", RESOURCE_FOLDER_PATH, vertexShaderName);

		ClassicPipelineStateGroupDesc desc = {};
		desc._vertexShaderFilePath = vertexShaderPath;
		desc._pixelShaderFilePath = pixelShaderPath;
		desc._depthComparisonFunc = COMPARISON_FUNC_LESS_EQUAL;
		*implDesc._classicPipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(desc, rootSignatureDesc);

		desc._pixelShaderFilePath = nullptr;
		*implDesc._classicDepthPipelineStateGroup = pipelineStateSystem->createPipelineStateGroup(desc, rootSignatureDesc);
	}

	// コマンドシグネチャ
	{
		GraphicsApiInstanceAllocator* allocator = GraphicsApiInstanceAllocator::Get();
		*implDesc._commandSignature = allocator->allocateCommandSignature();
		*implDesc._msCommandSignature = allocator->allocateCommandSignature();

		IndirectArgumentDesc argumentDescs[2] = {};
		argumentDescs[0]._type = INDIRECT_ARGUMENT_TYPE_CONSTANT;
		argumentDescs[0].Constant._num32BitValuesToSet = 4;
		argumentDescs[0].Constant._rootParameterIndex = DefaultMeshRootParam::INDIRECT_CONSTANT;
		argumentDescs[1]._type = INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH;

		CommandSignatureDesc desc = {};
		desc._device = device;
		desc._byteStride = sizeof(gpu::DispatchMeshIndirectArgument);

		desc._argumentDescs = argumentDescs;
		desc._numArgumentDescs = LTN_COUNTOF(argumentDescs);
		desc._rootSignature = (*implDesc._pipelineStateGroup)->getRootSignature();
		(*implDesc._commandSignature)->initialize(desc);

		desc._rootSignature = (*implDesc._primPipelineStateGroup)->getRootSignature();
		(*implDesc._msCommandSignature)->initialize(desc);
	}

#if ENABLE_MULTI_INDIRECT_DRAW
	{
		GraphicsApiInstanceAllocator* allocator = GraphicsApiInstanceAllocator::Get();

		IndirectArgumentDesc argumentDescs[2] = {};
		argumentDescs[0]._type = INDIRECT_ARGUMENT_TYPE_CONSTANT;
		argumentDescs[0].Constant._rootParameterIndex = ClassicMeshRootParam::MESH_INFO;
		argumentDescs[0].Constant._num32BitValuesToSet = 2;
		argumentDescs[1]._type = INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

		CommandSignatureDesc desc = {};
		desc._device = device;
		desc._byteStride = sizeof(gpu::StarndardMeshIndirectArguments);
		desc._argumentDescs = argumentDescs;
		desc._numArgumentDescs = LTN_COUNTOF(argumentDescs);
		desc._rootSignature = (*implDesc._classicPipelineStateGroup)->getRootSignature();
		(*implDesc._multiDrawCommandSignature) = allocator->allocateCommandSignature();
		(*implDesc._multiDrawCommandSignature)->initialize(desc);
	}
#endif
}

void ShaderSetImpl::terminate() {
	_shaderParamInstances.terminate();
	delete[] _parameterDatas;
	_parameterDatas = nullptr;
}

void PipelineStateSet::requestDelete(u32 shaderSetIndex) {
	if (_pipelineStateGroups[shaderSetIndex]) {
		_pipelineStateGroups[shaderSetIndex]->requestToDestroy();
		_pipelineStateGroups[shaderSetIndex] = nullptr;
	}

	if (_depthPipelineStateGroups[shaderSetIndex]) {
		_depthPipelineStateGroups[shaderSetIndex]->requestToDestroy();
		_depthPipelineStateGroups[shaderSetIndex] = nullptr;
	}

	if (_debugCullingPassPipelineStateGroups[shaderSetIndex]) {
		_debugCullingPassPipelineStateGroups[shaderSetIndex]->requestToDestroy();
		_debugCullingPassPipelineStateGroups[shaderSetIndex] = nullptr;
	}

	if (_debugMeshletPipelineStateGroups[shaderSetIndex]) {
		_debugMeshletPipelineStateGroups[shaderSetIndex]->requestToDestroy();
		_debugMeshletPipelineStateGroups[shaderSetIndex] = nullptr;
	}

	if (_debugLodLevelPipelineStateGroups[shaderSetIndex]) {
		_debugLodLevelPipelineStateGroups[shaderSetIndex]->requestToDestroy();
		_debugLodLevelPipelineStateGroups[shaderSetIndex] = nullptr;
	}

	if (_debugOcclusionPipelineStateGroups[shaderSetIndex]) {
		_debugOcclusionPipelineStateGroups[shaderSetIndex]->requestToDestroy();
		_debugOcclusionPipelineStateGroups[shaderSetIndex] = nullptr;
	}

	if (_debugDepthPipelineStateGroups[shaderSetIndex]) {
		_debugDepthPipelineStateGroups[shaderSetIndex]->requestToDestroy();
		_debugDepthPipelineStateGroups[shaderSetIndex] = nullptr;
	}

	if (_debugTexcoordsPipelineStateGroups[shaderSetIndex]) {
		_debugTexcoordsPipelineStateGroups[shaderSetIndex]->requestToDestroy();
		_debugTexcoordsPipelineStateGroups[shaderSetIndex] = nullptr;
	}

	if (_debugWireFramePipelineStateGroups[shaderSetIndex]) {
		_debugWireFramePipelineStateGroups[shaderSetIndex]->requestToDestroy();
		_debugWireFramePipelineStateGroups[shaderSetIndex] = nullptr;
	}

	if (_commandSignatures[shaderSetIndex]) {
		_commandSignatures[shaderSetIndex]->terminate();
		_commandSignatures[shaderSetIndex] = nullptr;
	}
}
