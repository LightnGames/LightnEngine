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
#include <ThirdParty/ImGui/imgui.h>
#include <Animation/SkeletalAnimation.h>
#include <Animation/BlendSpace1D.h>

RefPtr<SkyboxActor> sky;
RefPtr<SkeletalMeshActor> sk;
RefPtr<DirectionalLightActor> dirLight;

Scene::Scene() {

	sky = makeChild<SkyboxActor>();
	sk = makeChild<SkeletalMeshActor>();
	dirLight = makeChild<DirectionalLightActor>();

	sk->_camera = sk->addComponent<CameraComponent>();
	CameraComponent::mainCamera = sk->_camera;

	sk->_camera->setWorldRotation(Quaternion::euler({ 0, 0, 0 }));

	//�}�e���A���f�[�^
	std::vector<std::string> heromat;
	heromat.push_back("M_Hero.mat");
	heromat.push_back("M_Hero.mat");
	heromat.push_back("M_Hero_Crossbow.mat");

	sk->setUpSkeletalMesh("Hero/Hero.mesh", heromat);
	sk->setActorPosition({ 15,0.1f,0 });
	//sk2->setActorRotation(Quaternion::euler({ 15,0.1f,5 }));
	sk->setActorScale({ 0.015f,0.015f,0.015f });
	sk->_skeletalMeshComponent->setLocalRotation(Quaternion::euler({0,90,0}));

	auto& anim = sk->_animationComponent->_animationController;

	anim->addAnimationList<SkeletalAnimation>("Resources/Hero/Hero_idle.anim");
	anim->addAnimationList<SkeletalAnimation>("Resources/Hero/Hero_Run.anim");
	anim->addAnimationList<SkeletalAnimation>("Resources/Hero/Hero_StandHalfTurnRight.anim");

	auto blend = anim->addAnimationList<BlendSpace1D>("MovementBlend");
	blend->addBlendAnimation("Resources/Hero/Hero_idle.anim", 0.0f);
	blend->addBlendAnimation("Resources/Hero/Hero_StandHalfTurnLeft.anim", -1.0f);
	blend->addBlendAnimation("Resources/Hero/Hero_StandHalfTurnRight.anim", 1.0f);
	blend->setUpBlendSpace();
	blend->setPlayRate(1.0f);

	anim->applyRootMotion = true;
	anim->setRootMotionBone("Bip001 Pelvis");

	anim->play("MovementBlend");

	struct MeshNameAndMaterials {
		std::string objectName;
		std::vector<std::string> matFiles;
		std::vector<Matrix4> matrices;
	};

	std::unordered_map<std::string, MeshNameAndMaterials> instanceMatrices;

	//�V�[���t�@�C������I�u�W�F�N�g�̔z�u�����擾
	std::string fileName("Test.scene");
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
		} else {
			MeshNameAndMaterials key;
			key.objectName = std::string(objectName) + ".mesh";
			key.matFiles = std::move(materialNames);
			key.matrices.emplace_back(Matrix4::createWorldMatrix(position, rotation, scale));

			instanceMatrices[keyName] = key;
		}
	}

	//StaticInstanceMesh�̃Z�b�g�A�b�v
	uint32 maxDrawCount = 0;
	std::vector<uint32> indexList;

	for (auto&& ms : instanceMatrices) {

		const std::vector<Matrix4>& m = ms.second.matrices;

		auto mesh = makeChild<StaticInstanceMeshActor>();
		mesh->_staticInstanceMeshComponent->setUpMesh(ms.second.objectName,
			ms.second.matFiles, m,static_cast<uint32>(indexList.size()), maxDrawCount);

		for (auto&& m : mesh->_staticInstanceMeshComponent->meshInfo()->materialSlots) {
			indexList.push_back(m->faceCount * 3);
		}
		
		uint32 a = 4 - (m.size() % 4);
		if (a == 4) {
			a = 0;
		}

		maxDrawCount += (m.size() + a);
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

			pointLight->_lightComponent->light.color = Vector3(1, 1, 1)*intensity*5;
			pointLight->_lightComponent->light.attenuationEnd = range;
		}

		if (lightType == 0) {
			auto spotLight = makeChild<SpotLightActor>();
			spotLight->setActorPosition(position);
			spotLight->setActorRotation(rotation);

			spotLight->_lightComponent->light.color = Vector3(1, 1, 1)*intensity*5;
			spotLight->_lightComponent->light.attenuationEnd = range;
		}
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
	static float ccc = 2.0f;
	ccc -= 0.1f;
	if (ccc < 0) {
		//sk->_animationComponent->play("Hero_idle", 0.2f);
	}

	Quaternion cameraRotateYaw = Quaternion::euler({ 0, sk->_camera->getWorldRotation().getYaw(), 0 }, true);
	Vector3 differenceInputRotate = Quaternion::rotVector(sk->_skeletalMeshComponent->getLocalRotation().inverse()*cameraRotateYaw, velocity.normalize());
	//differenceInputRotate = velocity.normalize();
	float turnAmount = std::atan2(differenceInputRotate.z, -differenceInputRotate.x);
	//turnAmount += (std::atan2(velocity.z, -velocity.x)- turnAmount)/2.0f;
	//turnAmount = std::atan2(velocity.z, -velocity.x);
	auto blendAnim = sk->_animationComponent->_animationController->getAnimation<BlendSpace1D>("MovementBlend");

	if ((Vector3::length(velocity) < 0.01f) && walkAnim) {

		sk->_animationComponent->play("MovementBlend", 0.2f);
		//sk->_animationComponent->play("Hero_idle", 0.2f);
		//sk->_animationComponent->play("Hero_StandHalfTurnRight");
		walkAnim = false;
	}

	if (std::abs(turnAmount) < 0.3f) {
		if ((Vector3::length(velocity) > 0.01f) && !walkAnim) {
			sk->_animationComponent->play("Hero_Run", 0.2f);
			blendAnim->setBlendSpace(0);
			walkAnim = true;
		}

	}

	if (Vector3::length(velocity) < 0.01f) {
		turnAmount = 0;
	}

	blendAnim->setBlendSpace(lerp(0.1f, blendAnim->getBlendSpace(), turnAmount * 2));

	float f = blendAnim->getBlendSpace();
	ImGui::Begin("anim blend");
	ImGui::SliderFloat("Value", &f, -2, 2);
	ImGui::End();

	TransformQ rootMotionVelocity = sk->_animationComponent->_animationController->rootMotionVelocity;

	
	//��]�Ƀ��[�g���[�V������K�p���Ă��܂���90�x�␳���Ȃ���Z�𐳖ʂɌ����Ȃ�
	Vector3 moveVelocity = Quaternion::rotVector(sk->_skeletalMeshComponent->getWorldRotation()*Quaternion::euler({ 0,-90,0 }), rootMotionVelocity.position);
	if (velocity.length() > 0) {
		Quaternion inpuRotate = Quaternion::lookRotation(velocity.normalize());
		Quaternion smoothRotate = Quaternion::slerp(sk->_skeletalMeshComponent->getLocalRotation(), inpuRotate*cameraRotateYaw*Quaternion::euler({ 0,90,0 }), 0.1f);
		sk->_skeletalMeshComponent->setLocalRotation(smoothRotate);

		//moveVelocity = Quaternion::rotVector(sk->getActorRotation()*inpuRotate, rootMotionVelocity);
	}
	//sk2->_animationComponent->_animationController->debugTime = time;

	//sk->_skeletalMeshComponent->setLocalRotation(sk->_skeletalMeshComponent->getLocalRotation()*sk->_animationComponent->_animationController->rootMotionVelocity.rotation);
	//sk->_skeletalMeshComponent->setLocalRotation(Quaternion::euler({ 0,90,0 }));

	//sk->_skeletalMeshComponent->addLocalRotation(rootMotionVelocity.rotation);
	//sk->_skeletalMeshComponent->setLocalRotation(Quaternion::identity);


	moveVelocity.y = 0;
	moveVelocity *= sk->getActorScale().x;
	sk->setActorPosition(sk->getActorPosition() + moveVelocity);
	Quaternion rotate = Quaternion::euler({ turnVelocityP ,turnVelocity,0 });
	float cameraLength = 6;
	Vector3 cameraOffset = Quaternion::rotVector(rotate, Vector3::forward);
	sk->_camera->setLocalRotation(rotate);
	sk->_camera->setLocalPosition(-cameraOffset * cameraLength + Vector3(0.0f, 2.6f, 0));


	/*SceneRendererManager::debugDrawBox(sk->getActorPosition(), Vector3(1, 2, 1), sk->getActorRotation());
	SceneRendererManager::debugDrawLine(Vector3::zero, sk->getActorPosition());
	SceneRendererManager::debugDrawSphere(sk->getActorPosition() + Vector3::up, 0.5f);*/
}



Scene::~Scene() {
}
