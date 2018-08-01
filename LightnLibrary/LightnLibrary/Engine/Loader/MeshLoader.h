#pragma once

#include <d3d11.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <Util/ComPtr.h>
#include <Renderer/Mesh/Mesh.h>
#include <Renderer/Mesh/Skeleton.h>

enum class MeshType{
	None,
	Static,
	Skeletal,
	Animation
};

class MeshLoader{

public:

	MeshLoader(const ComPtr<ID3D11Device>& device,const ComPtr<ID3D11DeviceContext>& deviceContext);

	~MeshLoader();

	//FBX�t�@�C���ǂݍ���
	HRESULT load(const std::string& filePath, const std::vector<std::string>& matFiles);

	//�X�P���g���f�[�^�ǂݍ���
	std::unique_ptr<Skeleton> loadSkeleton();

	//���b�V���f�[�^���擾
	LocalMesh getMeshes() const;

	//���b�V���^�C�v���擾
	MeshType getType() const;

private:

	//�}�e���A���f�[�^����GPU�o�b�t�@�𐶐�
	HRESULT setupMaterial();

	//�}�e���A���f�[�^���t�@�C������ǂݍ���
	HRESULT loadMaterialFromFile(const std::vector<std::string>& fileName);

	//�V�F�[�_�[�ƒ萔�o�b�t�@�𐶐�
	HRESULT createShaderAndConstantBuffer(std::shared_ptr<MaterialData>& material, bool isSkeletalMesh);

	//���̃��b�V���ɂ�����t���p�X���擾
	std::string getFullPath(const char* fileName) const;

	//�X�P���^�����b�V�����H
	bool isSkeletalMesh();

	//�t�@�C�����烁�b�V���f�[�^�ǂݍ���
	void loadMaterialAndVertexData(std::ifstream& fin);

private:

	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11DeviceContext> _deviceContext;

	std::string _folderPath;
	std::string _filePath;

	//���b�V���f�[�^
	LocalMesh _mesh;

	MeshType _type;

	std::unique_ptr<Skeleton> _skeleton;

	int _materialSlotCount;
};