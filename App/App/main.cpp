#include <GameEngine.h>
#include <Scene/SceneManager.h>
#include <Actor/SkyboxActor.h>
#include <Actor/StaticMeshActor.h>
#include <Actor/StaticInstanceMeshActor.h>
#include <Actor/SkeletalMeshActor.h>
#include <Actor/DirectionalLightActor.h>
#include <Actor/PointLightActor.h>
#include <Actor/SpotLightActor.h>
#include <Actor/SkyboxActor.h>
#include <Renderer/SceneRendererManager.h>
#include <ThirdParty/ImGui/imgui.h>
#include <Animation/SkeletalAnimation.h>
#include <Animation/BlendSpace2D.h>
#include <Util/Input.h>

RefPtr<SkyboxActor> sky;
RefPtr<SkeletalMeshActor> sk;
RefPtr<DirectionalLightActor> dirLight;

class TestScene :public Scene {
public:

	virtual void start() override {

		sky = makeChild<SkyboxActor>();
		sk = makeChild<SkeletalMeshActor>();
		dirLight = makeChild<DirectionalLightActor>();

		sk->_camera = sk->addComponent<CameraComponent>();
		CameraComponent::mainCamera = sk->_camera;

		//マテリアルデータ
		std::vector<std::string> heromat;
		heromat.push_back("M_Hero.mat");
		heromat.push_back("M_Hero.mat");
		heromat.push_back("M_Hero_Crossbow.mat");

		sk->setUpSkeletalMesh("Hero/Hero.mesh", heromat);
		sk->setActorPosition({ 15, 0.1f, 0 });
		sk->setActorScale({ 0.015f, 0.015f, 0.015f });
		sk->_skeletalMeshComponent->setLocalRotation(Quaternion::euler( 0, 90, 0 ));

		auto& anim = sk->_animationComponent->_animationController;
		auto blend = anim->addAnimationList<BlendSpace2D>("MovementBlend");
		blend->addBlendAnimation("Resources/Hero/Hero_idle.anim", 0.0f, 0.0f);
		blend->addBlendAnimation("Resources/Hero/Hero_StandHalfTurnLeft.anim", -1.0f, 0.0f);
		blend->addBlendAnimation("Resources/Hero/Hero_StandHalfTurnRight.anim", 1.0f, 0.0f);
		blend->addBlendAnimation("Resources/Hero/Hero_Run.anim", 0.0f, 1.0f);
		blend->addBlendAnimation("Resources/Hero/Hero_RunRight.anim", 0.5f, 1.0f);
		blend->addBlendAnimation("Resources/Hero/Hero_RunLeft.anim", -0.5f, 1.0f);
		blend->setUpBlendSpace();
		blend->setPlayRate(1.0f);

		anim->applyRootMotion = true;
		anim->setRootMotionBone("Bip001 Pelvis");

		anim->play("MovementBlend");
		//anim->play("Hero_RunLeft");

		loadSceneAsset("Test.scene");
	}

	virtual void update(float deltaTime) override {
		GameTask::update(deltaTime);

		//入力を取得
		const float turnSpeed = 0.05f;
		static float turnVelocityYaw = 0;
		static float turnVelocityPitch = 0;
		static Vector3 inputVelocity = Vector3::zero;
		Vector3 inputDirectVelocity = Vector3::zero;

		if (Input::getKey(KeyCode::W)) {
			inputDirectVelocity += Vector3::forward;
		}

		if (Input::getKey(KeyCode::S)) {
			inputDirectVelocity -= Vector3::forward;
		}

		if (Input::getKey(KeyCode::D)) {
			inputDirectVelocity += Vector3::right;
		}

		if (Input::getKey(KeyCode::A)) {
			inputDirectVelocity -= Vector3::right;
		}

		if (Input::getKey(KeyCode::Right)) {
			turnVelocityYaw += 1;
		}

		if (Input::getKey(KeyCode::Left)) {
			turnVelocityYaw -= 1;
		}

		if (Input::getKey(KeyCode::Up)) {
			turnVelocityPitch += 1;
		}

		if (Input::getKey(KeyCode::Down)) {
			turnVelocityPitch -= 1;
		}

		//入力を滑らかに補完
		inputVelocity = Vector3::lerp(inputVelocity, inputDirectVelocity, 0.4f);

		//Animation再生ブレンド係数を計算
		const Quaternion skLocalRotateInverse = sk->_skeletalMeshComponent->getLocalRotation().inverse();
		const Quaternion cameraRotateYaw = Quaternion::euler( 0, sk->_camera->getWorldRotation().getYaw(), 0 , true);
		const Vector3 differenceInputLocalRotate = Quaternion::rotVector(skLocalRotateInverse*cameraRotateYaw, inputVelocity);
		const Vector3 differenceInputRotateWorld = Quaternion::rotVector(skLocalRotateInverse*Quaternion::euler( 0, 90, 0 )*cameraRotateYaw, inputVelocity);
		
		float turnAmount = std::atan2(differenceInputLocalRotate.z, -differenceInputLocalRotate.x);
		auto blendAnim = sk->_animationComponent->_animationController->getAnimation<BlendSpace2D>("MovementBlend");

		if (inputVelocity.length() < 0.01f) {
			turnAmount = 0;
		}

		blendAnim->setBlendSpace(turnAmount, differenceInputRotateWorld.z, 0.2f);

		//回転にルートモーションを適用してしまうと90度補正しないとZを正面に向かない
		const TransformQ rootMotionVelocity = sk->_animationComponent->_animationController->getRootMotionVelocity();
		const Quaternion forwardRotate = sk->_skeletalMeshComponent->getWorldRotation()*Quaternion::euler( 0, -90, 0 );
		const Vector3 moveVelocity = Quaternion::rotVector(forwardRotate, rootMotionVelocity.position)*sk->getActorScale().x;

		//入力があればその方向を向く
		if (inputVelocity.length() > 0) {
			const Quaternion inpuRotate = Quaternion::lookRotation(inputVelocity.normalize());
			const Quaternion smoothRotate = Quaternion::slerp(sk->_skeletalMeshComponent->getLocalRotation(), inpuRotate*cameraRotateYaw*Quaternion::euler( 0, 90, 0 ), turnSpeed);
			sk->_skeletalMeshComponent->setLocalRotation(smoothRotate);
		}

		//プレイヤーアクター移動
		//sk->_skeletalMeshComponent->addLocalRotation(rootMotionVelocity.rotation);
		sk->setActorPosition(sk->getActorPosition() + moveVelocity);

		//カメラ位置計算
		const Quaternion rotate = Quaternion::euler({ turnVelocityPitch, turnVelocityYaw, 0 });
		const float cameraLength = 7;
		const Vector3 cameraRotateOffset = Quaternion::rotVector(rotate, Vector3::forward);
		const Vector3 cameraTranslateOffset(0.0f, 2.5f, 0);
		sk->_camera->setLocalRotation(rotate);
		sk->_camera->setLocalPosition(-cameraRotateOffset * cameraLength + cameraTranslateOffset);

		//デバッグ描画
		const Vector3 skPosition = sk->_skeletalMeshComponent->getWorldPosition();
		SceneRendererManager::debugDrawLine(skPosition, skPosition + differenceInputRotateWorld);

		//SceneRendererManager::debugDrawBox(sk->getActorPosition(), Vector3(1, 2, 1), sk->getActorRotation());
		//SceneRendererManager::debugDrawLine(Vector3::zero, sk->getActorPosition());
		//SceneRendererManager::debugDrawSphere(sk->getActorPosition() + Vector3::up, 0.5f);
	}
};

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR szStr, int iCmdShow) {

	GameEngine* engine = new GameEngine();
	engine->initialize(hInst);
	engine->getSceneManager()->createScene<TestScene>();
	engine->run();

	return 0;
}