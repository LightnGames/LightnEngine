#include "Scene.h"
#include "Actor/SkyboxActor.h"
#include "Actor/StaticMeshActor.h"
#include "Actor/StaticInstanceMeshActor.h"
#include "Actor/SkeletalMeshActor.h"
#include <Actor/DirectionalLightActor.h>
#include <Actor/PointLightActor.h>
#include <Actor/SpotLightActor.h>
#include <Actor/SkyboxActor.h>
#include <Task/GameTask.h>
#include <Renderer/SceneRendererManager.h>

#include <Util/Input.h>
#include <iostream>
#include <fstream>
#include <assert.h>

#include <unordered_map>

RefPtr<SkyboxActor> sky;
std::vector<RefPtr<StaticMeshActor>> smTests;
RefPtr<SkeletalMeshActor> sk;
RefPtr<DirectionalLightActor> dirLight;
RefPtr<SpotLightActor> spotLight;
RefPtr<PointLightActor> pointLight;

Scene::Scene() {

	sky = makeChild<SkyboxActor>();
	sk = makeChild<SkeletalMeshActor>();
	dirLight = makeChild<DirectionalLightActor>();
	pointLight = makeChild<PointLightActor>();
	spotLight = makeChild<SpotLightActor>();

	//マテリアルデータ
	std::vector<std::string> skyMatFiles;
	skyMatFiles.push_back("SkyBoxMaterial.mat");
	sky->setUpStaticMesh("SkyBox.mesh", skyMatFiles);

	std::vector<std::string> lp287mat;
	lp287mat.push_back("M_LP287_Hair.mat");
	lp287mat.push_back("M_LP287_LegBelt.mat");
	lp287mat.push_back("M_LP287_BeltPelvis.mat");
	lp287mat.push_back("M_LP287_Skin.mat");
	lp287mat.push_back("M_LP287_Pants.mat");
	lp287mat.push_back("M_LP287_Jacet.mat");

	std::vector<std::string> zombieMat;
	zombieMat.push_back("SkeltalTest.mat");
	zombieMat.push_back("SkeltalTest.mat");
	/*zombieMat.push_back("SkeltalTest.mat");
	zombieMat.push_back("SkeltalTest.mat");*/
	sk->setUpSkeletalMesh("LP287/LP287.mesh", lp287mat);

	//_skeletalMesh = GraphicsResourceManager::instance().loadRenderableObject("Zombie.FBX", zombieMat).cast<SkeletalMesh>();

	//return;

	struct MeshNameAndMaterials {
		std::string objectName;
		std::vector<std::string> matFiles;
		std::vector<Matrix4> matrices;
	};

	std::unordered_map<std::string, MeshNameAndMaterials> instanceMatrices;

	//シーンファイルからオブジェクトの配置情報を取得
	std::string fileName("Test.scene");
	std::ifstream fin("Scene/" + fileName, std::ios::in | std::ios::binary);

	assert(!fin.fail());

	int32 objectCount;
	fin.read(reinterpret_cast<char*>(&objectCount), 4);

	for (int i = 0; i < objectCount; ++i) {
		
		char objectName[64] = {};
		int32 materialCount;
		std::vector<std::string> materialNames;
		std::string append;

		fin.read(reinterpret_cast<char*>(objectName), 64);
		fin.read(reinterpret_cast<char*>(&materialCount), 4);

		for (int j = 0; j < materialCount; ++j) {
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

		//auto mesh = makeChild<StaticMeshActor>();
		//smTests.emplace_back(mesh);

		//mesh->setActorPosition(position);
		//mesh->setActorScale(scale);
		//mesh->setActorRotation(rotation);

		//mesh->setUpStaticMesh(std::string(objectName) + ".mesh", materialNames);

		std::string keyName(objectName + append);

		if (instanceMatrices.count(keyName) > 0) {
			MeshNameAndMaterials& key = instanceMatrices[keyName];
			key.matrices.emplace_back(Matrix4::createWorldMatrix(position, rotation, scale));
		} else {
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
			ms.second.matFiles, m, indexList.size(), maxDrawCount);

		for (auto&& m : mesh->_staticInstanceMeshComponent->meshInfo()->materialSlots) {
			indexList.push_back(m->faceCount * 3);
		}
		
		uint32 a = 4 - (m.size() % 4);
		if (a == 4) {
			a = 0;
		}

		maxDrawCount += (m.size() + a);

	}

	fin.close();


	SceneRendererManager::instance().setUpStaticInstanceMeshRendere(maxDrawCount, indexList);
}

void Scene::update(float deltaTime)
{
	GameTask::update(deltaTime);

	static float count = 0;
	count += 0.10555f;
	Vector3 velocity = Vector3::zero;
	float speed = 0.05f;
	if (Input::getKey(KeyCode::W)) {
		velocity += Vector3::forward;
	}

	if (Input::getKey(KeyCode::S)) {
		velocity -= Vector3::forward;
	}

	if (Input::getKey(KeyCode::D)) {
		velocity += Vector3::right;
	}

	if (Input::getKey(KeyCode::A)) {
		velocity -= Vector3::right;
	}

	static float turnVelocity = 0;
	static float turnVelocityP = 0;
	if (Input::getKey(KeyCode::Right)) {
		turnVelocity += 1;
	}

	if (Input::getKey(KeyCode::Left)) {
		turnVelocity -= 1;
	}

	if (Input::getKey(KeyCode::Up)) {
		turnVelocityP += 1;
	}

	if (Input::getKey(KeyCode::Down)) {
		turnVelocityP -= 1;
	}

	velocity *= speed;

	static bool walkAnim = false;

	if ((Vector3::length(velocity) < 0.01f) && walkAnim) {
		sk->_animationComponent->play("ARIdle", 0.2f);
		walkAnim = false;
	}


	if ((Vector3::length(velocity) > 0.01f) && !walkAnim) {
		sk->_animationComponent->play("ARRun", 0.2f);
		walkAnim = true;
	}

	velocity = Quaternion::rotVector(sk->getActorRotation(), velocity);

	sk->_skeletalMeshComponent->setLocalRotation(Quaternion::euler({ 0, 90, 0}));
	sk->setActorScale({ 0.01f,0.01f,0.01f });
	sk->setActorPosition(sk->getActorPosition() + velocity);
	sk->setActorRotation(Quaternion::euler({ 0,turnVelocity,0 }));

	/*SceneRendererManager::debugDrawBox(sk->getActorPosition(), Vector3(1, 2, 1), sk->getActorRotation());
	SceneRendererManager::debugDrawLine(Vector3::zero, sk->getActorPosition());
	SceneRendererManager::debugDrawSphere(sk->getActorPosition() + Vector3::up, 0.5f);*/
}



Scene::~Scene() {
}
