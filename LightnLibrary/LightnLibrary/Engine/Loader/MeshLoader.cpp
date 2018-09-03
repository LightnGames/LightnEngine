#include "MeshLoader.h"
#include <Renderer/RendererUtil.h>
#include <Renderer/Mesh/SkeletalMesh.h>
#include <Renderer/GraphicsResourceManager.h>

#include <DirectXTex.h>
#include <iostream>
#include <fstream>
#include <cassert>

MeshLoader::MeshLoader(const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& deviceContext)
	:_device{ device }, _deviceContext{ deviceContext } {
}

MeshLoader::~MeshLoader() {
}

HRESULT MeshLoader::load(const std::string & filePath, const std::vector<std::string>& matFiles) {

	//���b�V���p�X���擾
	const int path_slash = filePath.find_last_of("/") + 1;
	_folderPath = "Resources/" + filePath.substr(0, path_slash);
	_filePath = filePath;

	const int path_point = filePath.find_last_of(".") + 1;
	auto fileName = filePath.substr(0, path_point) + "mesh";

	//���b�V���o�C�i�����[�h
	std::ifstream fin("Resources/" + fileName, std::ios::in | std::ios::binary);
	fin.exceptions(std::ios::badbit);
	
	if (fin.fail()) {
		int b = 0;
	}
	
	assert(!fin.fail());

	//�}�e���A���ƒ��_�o�b�t�@���[�h
	loadMaterialAndVertexData(fin);

	if (getType() == MeshType::Skeletal) {

		//�X�P���g�����[�h
		int gcnt = 0;
		int boneCount = 0;
		fin.read(reinterpret_cast<char*>(&boneCount), 4);
		gcnt = fin.gcount();

		_skeleton = std::make_unique<Skeleton>(boneCount);

		for (int i = 0; i < boneCount; i++) {
			char boneName[32] = {};
			fin.read(reinterpret_cast<char*>(boneName), 32);

			Matrix4 matrix;
			fin.read(reinterpret_cast<char*>(&matrix), 64);

			_skeleton->boneMatrices[i].name = boneName;
			_skeleton->boneMatrices[i].matrix = matrix;
		}
	}

	fin.close();

	//�}�e���A���f�[�^���t�@�C������ǂݍ���
	loadMaterialFromFile(matFiles);
	setupMaterial();

	return S_OK;
}

void MeshLoader::loadMaterialAndVertexData(std::ifstream& fin) {

	//���b�V���^�C�v
	char meshType[2];
	fin.read(reinterpret_cast<char*>(meshType), 2);

	//�}�e���A����
	int materialCount = 0;
	fin.read(reinterpret_cast<char*>(&materialCount), 4);

	//�}�e���A���̐������ǂݍ���
	for (int i = 0; i < materialCount; i++) {

		char materialName[32] = {};
		fin.read(reinterpret_cast<char*>(materialName), 32);

		int faceCount = 0;
		fin.read(reinterpret_cast<char*>(&faceCount), 4);

		int indexCount = 0;
		fin.read(reinterpret_cast<char*>(&indexCount), 4);

		int indexSize = indexCount * sizeof(int);
		std::unique_ptr<char> indexBuffer = std::unique_ptr<char>(new char[indexSize]);
		fin.read(reinterpret_cast<char*>(indexBuffer.get()), indexSize);

		auto material = std::make_shared<MaterialData>();
		RendererUtil::createIndexBuffer(indexBuffer.get(), indexCount, material->pIndexBuffer, _device);
		material->faceCount = faceCount;

		_mesh.materialSlots.emplace_back(std::move(material));
	}

	try {
		//���_�o�b�t�@�ǂݍ���
		int gcnt = 0;
		int vertexSize = 0;
		fin.read(reinterpret_cast<char*>(&vertexSize), 4);
		gcnt = fin.gcount();

		auto vertexBuf = std::unique_ptr<char>(new char[vertexSize]);
		fin.read(reinterpret_cast<char*>(vertexBuf.get()), vertexSize);
		gcnt = fin.gcount();

		char ppp[4];

		//�o�E���f�B���O�{�b�N�X�ǂݍ���
		Vector3 min;
		Vector3 max;
		//fin.read(reinterpret_cast<char*>(ppp), 4);
		gcnt = fin.gcount();
		fin.read(reinterpret_cast<char*>(&min), sizeof(Vector3));
		gcnt = fin.gcount();
		fin.read(reinterpret_cast<char*>(&max), sizeof(Vector3));
		gcnt = fin.gcount();

		_mesh.boundingBox.min = min;
		_mesh.boundingBox.max = max;

		//�X�^�e�B�b�N���b�V�����_�o�b�t�@����
		RendererUtil::createVertexBuffer(vertexBuf.get(), vertexSize, _mesh.vertexBuffer, _device);
	}
catch (std::ios_base::failure& e) {
	std::cerr << "file read error." << std::endl;
	}

	//���b�V�������������o�Ɋi�[
	_materialSlotCount = materialCount;
	if (meshType[0] == 'S'&&meshType[1] == 'M') {
		_type = MeshType::Static;
	} else if (meshType[0] == 'S'&&meshType[1] == 'K') {
		_type = MeshType::Skeletal;
	}
}

std::unique_ptr<Skeleton> MeshLoader::loadSkeleton() {
	return std::move(_skeleton);
}

HRESULT MeshLoader::setupMaterial() {

	for (int i = 0; i < _materialSlotCount; i++) {

		//�X�̃}�e���A���|�C���^�𐶐�
		auto& material = _mesh.materialSlots[i];

		//�V�F�[�_�[�ƒ萔�o�b�t�@�𐶐�
		createShaderAndConstantBuffer(material, isSkeletalMesh());

		//�e�N�X�`���̃V�F�[�_�[���\�[�X�r���[���쐬
		for (UINT j = 0; j < material->textureCount; ++j) {

			//�e�N�X�`�����擾
			const std::string fullPath = _folderPath + material->textureFileNames[j];
			auto texture = GraphicsResourceManager::instance().loadTexture(fullPath);

			//�~�b�v�}�b�v�𐶐�
			_deviceContext->GenerateMips(texture.Get());

			//�}�e���A���ɓo�^
			material->ppTextures.emplace_back(texture);
		}

		//�e�N�X�`���\�T���v���[���쐬
		material->pSamplerLiner = GraphicsResourceManager::instance().simpleSamplerState();
	}

	return S_OK;
}

HRESULT MeshLoader::loadMaterialFromFile(const std::vector<std::string>& fileName) {

	//�}�e���A���̐���������
	for (int matIndex = 0; matIndex < fileName.size(); matIndex++) {

		std::string fullPath = _folderPath.c_str();
		fullPath += fileName[matIndex];
		std::ifstream ifs(fullPath);
		std::string str;

		auto& material = _mesh.materialSlots[matIndex];

		int textureCount = 0;
		int loadTextureCount = 0;
		int lineCount = 0;

		ifs.clear();
		ifs.seekg(0, std::ios::beg);

		//�t�@�C���̍s���Ƃɏ���
		while (getline(ifs, str)) {

			switch (lineCount) {
			case 0:
				//���_�V�F�[�_�[�t�@�C�����擾
				material->vertexShaderFileName = std::string(str.c_str());
				break;
			case 1:
				//�s�N�Z���V�F�[�_�[���擾
				material->pixelShaderFileName = std::string(str.c_str());
				break;
			case 2:
				//�}�e���A���A���t�@�^�C�v���擾
				material->alphaType = std::stoi(str);
				break;
			case 3:
				//�|���S���\�����[�h
				switch (std::stoi(str)) {
				case 0: material->cullMode = D3D11_CULL_BACK; break;
				case 1: material->cullMode = D3D11_CULL_NONE; break;
				case 2: material->cullMode = D3D11_CULL_FRONT; break;
				}
				break;
			case 4:
				//���̃}�e���A���̃e�N�X�`�������擾
				textureCount = std::stoi(str);
				material->textureCount = textureCount;
				break;
			default:
				//�e�N�X�`���̐������t�@�C������ǂݍ���
				if (loadTextureCount < textureCount) {
					material->textureFileNames.push_back(std::string(str.c_str()));
					loadTextureCount++;
				}
				break;
			}

			lineCount++;
		}
	}

	return S_OK;
}

HRESULT MeshLoader::createShaderAndConstantBuffer(std::shared_ptr<MaterialData>& material, bool isSkeletalMesh) {

	//���_�C���v�b�g���C�A�E�g���쐬
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout;
	layout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	layout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	layout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	layout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	layout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 });


	if (isSkeletalMesh) {
		layout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "BONE_INDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 });
		layout.emplace_back(D3D11_INPUT_ELEMENT_DESC{ "BONE_WEIGHT",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,72,D3D11_INPUT_PER_VERTEX_DATA,0 });
	}


	//���_�V�F�[�_�[��ǂݍ���
	RendererUtil::createVertexShader(material->vertexShaderFileName.c_str(), material->pVertexShader, layout.data(), layout.size(), material->pVertexLayout, _device);

	//�R���X�^���g�o�b�t�@�[�쐬
	const UINT constantBufferSize = isSkeletalMesh ? sizeof(SkeletalMeshConstantBuffer) : sizeof(MeshConstantBuffer);
	RendererUtil::createConstantBuffer(material->pConstantBuffer, constantBufferSize, _device);

	//�s�N�Z���V�F�[�_�[�ǂݍ���
	RendererUtil::createPixelShader(material->pixelShaderFileName.c_str(), material->pPixelShader, _device);

	return E_NOTIMPL;
}

std::string MeshLoader::getFullPath(const char * fileName) const {
	std::string fullPath(fileName);
	fullPath = _folderPath + fileName;

	return fullPath;
}

bool MeshLoader::isSkeletalMesh() {
	return _type == MeshType::Skeletal;
}

LocalMesh MeshLoader::getMeshes() const {
	return std::move(_mesh);
}

MeshType MeshLoader::getType() const {
	return _type;
}