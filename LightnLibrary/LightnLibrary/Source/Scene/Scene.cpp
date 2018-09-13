#include <Scene/Scene.h>
#include <Actor/SkyboxActor.h>
#include <Actor/StaticMeshActor.h>
#include <Actor/StaticInstanceMeshActor.h>
#include <Actor/SkeletalMeshActor.h>
#include <Actor/DirectionalLightActor.h>
#include <Actor/PointLightActor.h>
#include <Actor/SpotLightActor.h>
#include <Actor/SkyboxActor.h>
#include <Task/GameTask.h>
#include <Renderer/SceneRendererManager.h>

#include <iostream>
#include <fstream>
#include <assert.h>

#include <unordered_map>

Scene::Scene() {
}

void Scene::update(float deltaTime) {
}

void Scene::loadSceneAsset(const std::string & fileName) {

	struct MeshNameAndMaterials {
		std::string objectName;
		std::vector<std::string> matFiles;
		std::vector<Matrix4> matrices;
	};

	std::unordered_map<std::string, MeshNameAndMaterials> instanceMatrices;

	//シーンファイルからオブジェクトの配置情報を取得
	std::ifstream fin("Scene/" + fileName, std::ios::in | std::ios::binary);

	assert(!fin.fail());

	uint32 objectCount;
	fin.read(reinterpret_cast<char*>(&objectCount), 4);

	for (uint32 i = 0; i < objectCount; ++i) {

		char objectName[64] = {};
		uint32 materialCount;
		std::vector<std::string> materialNames;
		std::string append;

		fin.read(reinterpret_cast<char*>(objectName), 64);
		fin.read(reinterpret_cast<char*>(&materialCount), 4);

		for (uint32 j = 0; j < materialCount; ++j) {
			char materialName[32] = {};
			fin.read(reinterpret_cast<char*>(materialName), 32);
			materialNames.emplace_back(std::string(materialName) + ".mat");
			append += "_" + std::string(materialName);
		}

		Vector3 position;
		Quaternion rotation;
		Vector3 scale;

		fin.read(reinterpret_cast<char*>(&position), 12);
		fin.read(reinterpret_cast<char*>(&rotation), 16);
		fin.read(reinterpret_cast<char*>(&scale), 12);

		const std::string keyName(objectName + append);

		if (instanceMatrices.count(keyName) > 0) {
			MeshNameAndMaterials& key = instanceMatrices[keyName];
			key.matrices.emplace_back(Matrix4::createWorldMatrix(position, rotation, scale));
		}
		else {
			MeshNameAndMaterials key;
			key.objectName = std::string(objectName) + ".mesh";
			key.matFiles = std::move(materialNames);
			key.matrices.emplace_back(Matrix4::createWorldMatrix(position, rotation, scale));

			instanceMatrices[keyName] = key;
		}
	}

	//StaticInstanceMeshのセットアップ
	uint32 maxDrawCount = 0;
	std::vector<uint32> indexList;

	for (auto&& ms : instanceMatrices) {

		const std::vector<Matrix4>& m = ms.second.matrices;

		auto mesh = makeChild<StaticInstanceMeshActor>();
		mesh->_staticInstanceMeshComponent->setUpMesh(ms.second.objectName,
			ms.second.matFiles, m, static_cast<uint32>(indexList.size()), maxDrawCount);

		for (auto&& m : mesh->_staticInstanceMeshComponent->meshInfo()->materialSlots) {
			indexList.push_back(m->faceCount * 3);
		}

		uint32 a = 4 - (m.size() % 4);
		if (a == 4) {
			a = 0;
		}

		maxDrawCount += static_cast<uint32>(m.size() + a);
	}

	int32 lightCount;
	fin.read(reinterpret_cast<char*>(&lightCount), 4);

	for (int i = 0; i < lightCount; ++i) {

		int lightType;
		float intensity;
		float range;
		Vector3 position;
		Quaternion rotation;

		fin.read(reinterpret_cast<char*>(&lightType), 4);
		fin.read(reinterpret_cast<char*>(&intensity), 4);
		fin.read(reinterpret_cast<char*>(&range), 4);

		fin.read(reinterpret_cast<char*>(&position), 12);
		fin.read(reinterpret_cast<char*>(&rotation), 16);

		if (lightType == 2) {
			auto pointLight = makeChild<PointLightActor>();
			pointLight->setActorPosition(position);
			pointLight->setActorRotation(rotation);

			pointLight->_lightComponent->light.color = Vector3(1, 1, 1)*intensity * 5;
			pointLight->_lightComponent->light.attenuationEnd = range;
		}

		if (lightType == 0) {
			auto spotLight = makeChild<SpotLightActor>();
			spotLight->setActorPosition(position);
			spotLight->setActorRotation(rotation);

			spotLight->_lightComponent->light.color = Vector3(1, 1, 1)*intensity * 5;
			spotLight->_lightComponent->light.attenuationEnd = range;
		}
	}

	fin.close();


	SceneRendererManager::instance().setUpStaticInstanceMeshRendere(maxDrawCount, indexList);
}



Scene::~Scene() {
}
