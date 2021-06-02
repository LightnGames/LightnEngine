#include "PipelineStateSystem.h"
#include <GfxCore/impl/GraphicsSystemImpl.h>
#include <GfxCore/impl/GraphicsApiInterface.h>
#include <MeshRenderer/GpuStruct.h>

PipelineStateSystem _pipelineStateSystem;

void PipelineStateSystem::initialize() {
    _pipelineStates.initialize(PIPELINE_STATE_GROUP_COUNT_MAX);
    _sharedRootsignatures.initialize(PIPELINE_STATE_GROUP_COUNT_MAX);
}

void PipelineStateSystem::update() {
}

void PipelineStateSystem::processDeletion() {
    u32 pipelineStateCount = _pipelineStates.getResarveCount();
    for (u32 pipelineStateIndex = 0; pipelineStateIndex < pipelineStateCount; ++pipelineStateIndex) {
        if (_stateFlags[pipelineStateIndex] & PIPELINE_STATE_GROUP_FLAG_REQUEST_DESTROY) {
            _pipelineStateHashes[pipelineStateIndex] = 0;
            _stateFlags[pipelineStateIndex] = PIPELINE_STATE_GROUP_FLAG_NONE;
			PipelineStateGroup& pipelineStateGroup = _pipelineStates[pipelineStateIndex];
			SharedRootSignature* sharedRootSignature = pipelineStateGroup.getSharedRootSignature();
			u32 remainingRefCount = --sharedRootSignature->_refCount;
            LTN_ASSERT(remainingRefCount != gpu::INVALID_INDEX);
			if (remainingRefCount == 0) {
				sharedRootSignature->_rootSignature->terminate();
				u32 sharedRootSignatureIndex = static_cast<u32>(sharedRootSignature - &_sharedRootsignatures[0]);
				_sharedRootsignatures.discard(sharedRootSignatureIndex);
				_sharedRootSignatureHashes[sharedRootSignatureIndex] = 0;
			}

			pipelineStateGroup.terminate();
            _pipelineStates.discard(pipelineStateIndex);
        }
    }
}

void PipelineStateSystem::terminate() {
    LTN_ASSERT(_pipelineStates.getInstanceCount() == 0);
    LTN_ASSERT(_sharedRootsignatures.getInstanceCount() == 0);
    _pipelineStates.terminate();
    _sharedRootsignatures.terminate();
}

u32 PipelineStateSystem::getGroupIndex(const PipelineStateGroup* pipelineState) const {
    u32 index = static_cast<u32>(pipelineState - &_pipelineStates[0]);
    LTN_ASSERT(index < _pipelineStates.getResarveCount());
    return index;
}

PipelineStateGroup* PipelineStateSystem::createPipelineStateGroup(const MeshShaderPipelineStateGroupDesc& desc, const RootSignatureDesc& rootSignatureDesc) {
    u64 meshShaderHash = StrHash(desc._meshShaderFilePath);
    u64 amplificationShaderHash = 0;
    if (desc._amplificationShaderFilePath != nullptr) {
        amplificationShaderHash = StrHash(desc._amplificationShaderFilePath);
    }

    u64 pixelShaderHash = 0;
	if (desc._pixelShaderFilePath != nullptr) {
		pixelShaderHash = StrHash(desc._pixelShaderFilePath);
	}

	u64 rootsignatureDescHash = createRootSignatureDescHash(rootSignatureDesc);
	u32 findRootSignatureIndex = findSharedRootsignature(rootsignatureDescHash);
	if (findRootSignatureIndex == gpu::INVALID_INDEX) {
		findRootSignatureIndex = createSharedRootSignature(rootSignatureDesc);
		_sharedRootSignatureHashes[findRootSignatureIndex] = rootsignatureDescHash;
	}

	u64 shaderHash = meshShaderHash + amplificationShaderHash + pixelShaderHash;
    u32 findPipelineStateIndex = findPipelineStateGroup(shaderHash);
    if (findPipelineStateIndex == gpu::INVALID_INDEX) {
        findPipelineStateIndex = _pipelineStates.request();
		SharedRootSignature* sharedRootSignature = &_sharedRootsignatures[findRootSignatureIndex];
        PipelineStateGroup* pipelineState = &_pipelineStates[findPipelineStateIndex];
		pipelineState->initialize(desc, sharedRootSignature);
        pipelineState->setStateFlags(&_stateFlags[findPipelineStateIndex]);
        _pipelineStateHashes[findPipelineStateIndex] = shaderHash;

        sharedRootSignature->_refCount++;
    }

    PipelineStateGroup* pipelineState = &_pipelineStates[findPipelineStateIndex];
    return pipelineState;
}

PipelineStateGroup* PipelineStateSystem::createPipelineStateGroup(const ClassicPipelineStateGroupDesc& desc, const RootSignatureDesc& rootSignatureDesc) {
    u64 vertexShaderHash = StrHash(desc._vertexShaderFilePath);
    u64 pixelShaderHash = 0;
    if (desc._pixelShaderFilePath != nullptr) {
        pixelShaderHash = StrHash(desc._pixelShaderFilePath);
    }

	u64 rootsignatureDescHash = createRootSignatureDescHash(rootSignatureDesc);
	u32 findRootSignatureIndex = findSharedRootsignature(rootsignatureDescHash);
	if (findRootSignatureIndex == gpu::INVALID_INDEX) {
		findRootSignatureIndex = createSharedRootSignature(rootSignatureDesc);
		_sharedRootSignatureHashes[findRootSignatureIndex] = rootsignatureDescHash;
	}

    u64 shaderHash = vertexShaderHash + pixelShaderHash;
    u32 findIndex = findPipelineStateGroup(shaderHash);
    if (findIndex == gpu::INVALID_INDEX) {
        findIndex = _pipelineStates.request();
		SharedRootSignature* sharedRootSignature = &_sharedRootsignatures[findRootSignatureIndex];
        PipelineStateGroup* pipelineState = &_pipelineStates[findIndex];
        pipelineState->initialize(desc, sharedRootSignature);
        pipelineState->setStateFlags(&_stateFlags[findIndex]);
        _pipelineStateHashes[findIndex] = shaderHash;

        sharedRootSignature->_refCount++;
    }

    PipelineStateGroup* pipelineState = &_pipelineStates[findIndex];
    return pipelineState;
}

PipelineStateSystem* PipelineStateSystem::Get() {
    return &_pipelineStateSystem;
}

u32 PipelineStateSystem::createSharedRootSignature(const RootSignatureDesc& desc) {
	u32 allocateIndex = _sharedRootsignatures.request();
	LTN_ASSERT(_sharedRootSignatureHashes[allocateIndex] == 0);

	GraphicsApiInstanceAllocator* allocator = GraphicsApiInstanceAllocator::Get();
	SharedRootSignature* sharedRootSignature = &_sharedRootsignatures[allocateIndex];
	LTN_ASSERT(sharedRootSignature->_refCount == 0);

	sharedRootSignature->_rootSignature = allocator->allocateRootSignature();
	sharedRootSignature->_rootSignature->iniaitlize(desc);
	return allocateIndex;
}

u64 PipelineStateSystem::createRootSignatureDescHash(const RootSignatureDesc& desc) const {
	return BinHash(desc._parameters, sizeof(RootParameter)*desc._numParameters);
}

u32 PipelineStateSystem::findPipelineStateGroup(u64 hash) const {
    u32 findIndex = gpu::INVALID_INDEX;
    u32 pipelineStateCount = _pipelineStates.getResarveCount();
    for (u32 pipelineStateIndex = 0; pipelineStateIndex < pipelineStateCount; ++pipelineStateIndex) {
        if (_pipelineStateHashes[pipelineStateIndex] == hash) {
            findIndex = pipelineStateIndex;
            break;
        }
    }

    return findIndex;
}

u32 PipelineStateSystem::findSharedRootsignature(u64 hash) const {
	u32 findIndex = gpu::INVALID_INDEX;
	u32 sharedRootsignatureCount = _sharedRootsignatures.getResarveCount();
	for (u32 sharedRootsignatureIndex = 0; sharedRootsignatureIndex < sharedRootsignatureCount; ++sharedRootsignatureIndex) {
		if (_sharedRootSignatureHashes[sharedRootsignatureIndex] == hash) {
			findIndex = sharedRootsignatureIndex;
			break;
		}
	}

	return findIndex;
}

void PipelineStateGroup::initialize(const MeshShaderPipelineStateGroupDesc& desc, SharedRootSignature* sharedRootSignature) {
#if ENABLE_MESH_SHADER
	Device* device = GraphicsSystemImpl::Get()->getDevice();
	GraphicsApiInstanceAllocator* allocator = GraphicsApiInstanceAllocator::Get();

    ShaderBlob* meshShader = allocator->allocateShaderBlob();
    ShaderBlob* amplificationShader = nullptr;
    ShaderBlob* pixelShader = nullptr;
    ShaderByteCode amplificationShaderByteCode = {};
	ShaderByteCode pixelShaderByteCode = {};
    meshShader->initialize(desc._meshShaderFilePath);
    if (desc._amplificationShaderFilePath != nullptr) {
        amplificationShader = allocator->allocateShaderBlob();
        amplificationShader->initialize(desc._amplificationShaderFilePath);
        amplificationShaderByteCode = amplificationShader->getShaderByteCode();
    }

	if (desc._pixelShaderFilePath != nullptr) {
        pixelShader = allocator->allocateShaderBlob();
		pixelShader->initialize(desc._pixelShaderFilePath);
		pixelShaderByteCode = pixelShader->getShaderByteCode();
	}

    MeshPipelineStateDesc pipelineStateDesc = {};
    pipelineStateDesc._device = device;
    pipelineStateDesc._ms = meshShader->getShaderByteCode();
    pipelineStateDesc._as = amplificationShaderByteCode;
	pipelineStateDesc._ps = pixelShaderByteCode;
    pipelineStateDesc._numRenderTarget = 1;
    pipelineStateDesc._rtvFormats[0] = FORMAT_R8G8B8A8_UNORM;
    pipelineStateDesc._topologyType = PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateDesc._rootSignature = sharedRootSignature->_rootSignature;
    pipelineStateDesc._sampleDesc._count = 1;
    pipelineStateDesc._dsvFormat = FORMAT_D32_FLOAT;
	pipelineStateDesc._depthComparisonFunc = desc._depthComparisonFunc;
    pipelineStateDesc._blendDesc = desc._blendDesc;
	pipelineStateDesc._fillMode = desc._fillMode;
    _pipelineState = allocator->allocatePipelineState();
    _pipelineState->iniaitlize(pipelineStateDesc);
    _pipelineState->setDebugName(desc._meshShaderFilePath);
	_sharedRootSignature = sharedRootSignature;

    meshShader->terminate();
    if (amplificationShader != nullptr) {
        amplificationShader->terminate();
    }

	if (pixelShader != nullptr) {
		pixelShader->terminate();
	}
#endif
}

void PipelineStateGroup::initialize(const ClassicPipelineStateGroupDesc& desc, SharedRootSignature* sharedRootSignature) {
    Device* device = GraphicsSystemImpl::Get()->getDevice();
    GraphicsApiInstanceAllocator* allocator = GraphicsApiInstanceAllocator::Get();

    ShaderBlob* pixelShader = nullptr;
    ShaderBlob* vertexShader = allocator->allocateShaderBlob();
    vertexShader->initialize(desc._vertexShaderFilePath);

    ShaderByteCode pixelShaderByteCode = {};
    if (desc._pixelShaderFilePath != nullptr) {
        pixelShader = allocator->allocateShaderBlob();
        pixelShader->initialize(desc._pixelShaderFilePath);
        pixelShaderByteCode = pixelShader->getShaderByteCode();;
    }

    InputElementDesc inputElements[2] = {};
    inputElements[0]._inputSlot = 0;
    inputElements[0]._format = FORMAT_R32G32B32_FLOAT;
    inputElements[0]._semanticName = "POSITION";

    inputElements[1]._inputSlot = 1;
    inputElements[1]._format = FORMAT_R32_UINT;
    inputElements[1]._semanticName = "PACKED_TEX";

    GraphicsPipelineStateDesc pipelineStateDesc = {};
    pipelineStateDesc._device = device;
    pipelineStateDesc._vs = vertexShader->getShaderByteCode();
    pipelineStateDesc._ps = pixelShaderByteCode;
    pipelineStateDesc._numRenderTarget = 1;
    pipelineStateDesc._rtvFormats[0] = FORMAT_R8G8B8A8_UNORM;
    pipelineStateDesc._topologyType = PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateDesc._rootSignature = sharedRootSignature->_rootSignature;
    pipelineStateDesc._sampleDesc._count = 1;
    pipelineStateDesc._dsvFormat = FORMAT_D32_FLOAT;
    pipelineStateDesc._depthComparisonFunc = COMPARISON_FUNC_LESS_EQUAL;
    pipelineStateDesc._inputElements = inputElements;
    pipelineStateDesc._inputElementCount = LTN_COUNTOF(inputElements);
    _pipelineState = allocator->allocatePipelineState();
    _pipelineState->iniaitlize(pipelineStateDesc);
	_sharedRootSignature = sharedRootSignature;

    vertexShader->terminate();
    if (pixelShader != nullptr) {
        pixelShader->terminate();
    }
}

u64 PipelineStateSystem::getPipelineStateGrpupHash(const PipelineStateGroup* group) const {
    u32 index = getGroupIndex(group);
    return _pipelineStateHashes[index];
}

void PipelineStateGroup::terminate() {
	if (_pipelineState != nullptr) {
		_pipelineState->terminate();
	}
}

void PipelineStateGroup::requestToDestroy() {
    *_stateFlags |= PIPELINE_STATE_GROUP_FLAG_REQUEST_DESTROY;
}
