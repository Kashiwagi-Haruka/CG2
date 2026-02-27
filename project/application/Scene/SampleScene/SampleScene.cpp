#include "SampleScene.h"
#include "Function.h"
#include "GameBase.h"
#include "Input.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include "ParticleManager.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include <numbers>
#include <utility>
SampleScene::SampleScene() {

	uvBallObj_ = std::make_unique<Object3d>();
	fieldObj_ = std::make_unique<Object3d>();
	planeGltf_ = std::make_unique<Object3d>();
	animatedCubeObj_ = std::make_unique<Object3d>();
	humanObj_ = std::make_unique<Object3d>();
	portalA_ = std::make_unique<Primitive>();
	portalB_ = std::make_unique<Primitive>();
	portalRingA_ = std::make_unique<Primitive>();
	portalRingB_ = std::make_unique<Primitive>();
	spherePrimitive_ = std::make_unique<Primitive>();
	cameraTransform_ = {
	    .scale{0.1f, 0.1f, 0.1f  },
        .rotate{0.0f, 0.0f, 0.0f  },
        .translate{0.0f, 5.0f, -10.0f}
    };

	camera_ = std::make_unique<Camera>();
	portalCameraFromA_ = std::make_unique<Camera>();
	portalCameraFromB_ = std::make_unique<Camera>();
	debugCamera_ = std::make_unique<DebugCamera>();
	camera_->SetTransform(cameraTransform_);

	ModelManager::GetInstance()->LoadModel("Resources/3d", "uvBall");
	ModelManager::GetInstance()->LoadModel("Resources/3d", "terrain");
	ModelManager::GetInstance()->LoadGltfModel("Resources/3d", "planeG");
	ModelManager::GetInstance()->LoadGltfModel("Resources/3d/AnimatedCube", "AnimatedCube");
	ModelManager::GetInstance()->LoadGltfModel("Resources/3d/human", "walk");
	ModelManager::GetInstance()->LoadGltfModel("Resources/3d/human", "sneakWalk");
	ParticleManager::GetInstance()->CreateParticleGroup("sample", "Resources/2d/defaultParticle.png");
}
void SampleScene::Initialize() {

	debugCamera_->Initialize();
	debugCamera_->SetTranslation(cameraTransform_.translate);
	uvBallObj_->Initialize();
	uvBallObj_->SetCamera(camera_.get());
	uvBallObj_->SetModel("uvBall");
	fieldObj_->Initialize();
	fieldObj_->SetCamera(camera_.get());
	fieldObj_->SetModel("terrain");
	planeGltf_->Initialize();
	planeGltf_->SetCamera(camera_.get());
	planeGltf_->SetModel("planeG");
	animatedCubeObj_->Initialize();
	animatedCubeObj_->SetCamera(camera_.get());
	animatedCubeObj_->SetModel("AnimatedCube");
	humanObj_->Initialize();
	humanObj_->SetCamera(camera_.get());
	humanObj_->SetModel("walk");
	portalA_->Initialize(Primitive::Circle, 48);
	portalA_->SetCamera(camera_.get());
	portalA_->SetColor({0.3f, 0.7f, 1.0f, 1.0f});
	portalA_->SetEnableLighting(false);
	portalB_->Initialize(Primitive::Circle, 48);
	portalB_->SetCamera(camera_.get());
	portalB_->SetEnableLighting(false);
	portalRingA_->Initialize(Primitive::Ring, "Resources/TD3_3102/2d/ring.png", 48);
	portalRingA_->SetCamera(camera_.get());
	portalRingA_->SetEnableLighting(false);
	portalRingA_->SetColor({0.3f, 0.7f, 1.0f, 1.0f});
	portalRingB_->Initialize(Primitive::Ring, "Resources/TD3_3102/2d/ring.png", 48);
	portalRingB_->SetCamera(camera_.get());
	portalRingB_->SetEnableLighting(false);
	portalRingB_->SetColor({1.0f, 0.55f, 0.1f, 1.0f});
	spherePrimitive_->Initialize(Primitive::Sphere, 32);
	spherePrimitive_->SetCamera(camera_.get());
	spherePrimitive_->SetEnableLighting(true);
	uvBallTransform_ = {
	    .scale{1.0f, 1.0f, 1.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{0.0f, 0.0f, 0.0f}
    };
	planeGTransform_ = {
	    .scale{1.0f, 1.0f, 1.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{0.0f, 1.0f, 0.0f}
    };
	animatedCubeTransform_ = {
	    .scale{1.0f, 1.0f, 1.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{3.0f, 1.0f, 0.0f}
    };
	humanTransform_ = {
	    .scale{100.0f,	                        100.0f,                    100.0f},
        .rotate{-std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float>, 0.0f  },
        .translate{0.0f,                              1.0f,                      -3.0f }
    };
	particleTransform_ = {
	    .scale{0.1f, 0.1f, 0.1f },
        .rotate{0.0f, 0.0f, 0.0f },
        .translate{0.0f, 1.0f, -3.0f}
    };
	portalATransform_ = {
	    .scale{1.8f,  1.8f,                      1.0f},
        .rotate{0.0f,  std::numbers::pi_v<float>, 0.0f},
        .translate{-3.0f, 1.5f,                      2.0f}
    };
	portalBTransform_ = {
	    .scale{1.8f, 1.8f, 1.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{3.0f, 1.5f, 2.0f}
    };
	sampleParticleEmitter_ = std::make_unique<ParticleEmitter>("sample", particleTransform_, 0.1f, 5, Vector3{0.0f, 0.0f, 0.0f}, Vector3{-0.5f, -0.5f, -0.5f}, Vector3{0.5f, 0.5f, 0.5f});
	planeGltf_->SetTransform(planeGTransform_);
	animatedCubeAnimation_ = Animation::LoadAnimationData("Resources/3d/AnimatedCube", "AnimatedCube");
	animatedCubeObj_->SetAnimation(&animatedCubeAnimation_, true);
	animatedCubeObj_->SetTransform(animatedCubeTransform_);
	humanAnimationClips_ = Animation::LoadAnimationClips("Resources/3d/human", "walk");
	std::vector<Animation::AnimationData> sneakClips = Animation::LoadAnimationClips("Resources/3d/human", "sneakWalk");
	humanAnimationClips_.insert(humanAnimationClips_.end(), sneakClips.begin(), sneakClips.end());
	if (!humanAnimationClips_.empty()) {
		currentHumanAnimationIndex_ = 0;
		humanObj_->SetAnimation(&humanAnimationClips_[currentHumanAnimationIndex_], true);
	}
	humanObj_->SetTransform(humanTransform_);
	if (Model* walkModel = ModelManager::GetInstance()->FindModel("walk")) {
		humanSkeleton_ = std::make_unique<Skeleton>(Skeleton().Create(walkModel->GetModelData().rootnode));
		humanSkinCluster_ = CreateSkinCluster(*humanSkeleton_, *walkModel);
		if (!humanSkinCluster_.mappedPalette.empty()) {
			humanObj_->SetSkinCluster(&humanSkinCluster_);
		}
	}

	uvSprite = std::make_unique<Sprite>();
	uvSprite->Initialize(TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/uvChecker.png"));
	uvSprite->SetScale(Vector2(100, 100));
	uvSprite->SetRotation(0);
	uvSprite->SetPosition(Vector2(0, 0));

	portalA_->SetTransform(portalATransform_);
	portalB_->SetTransform(portalBTransform_);

	portalRingA_->SetTransform(portalATransform_);
	portalRingB_->SetTransform(portalBTransform_);

	portalRenderTextureA_ = std::make_unique<RenderTexture2D>();
	portalRenderTextureA_->Initialize(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, {0.05f, 0.05f, 0.1f, 1.0f});
	if (portalRenderTextureA_->IsReady()) {
		portalA_->SetTextureIndex(portalRenderTextureA_->GetSrvIndex());
	}
	portalRenderTextureB_ = std::make_unique<RenderTexture2D>();
	portalRenderTextureB_->Initialize(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, {0.05f, 0.05f, 0.1f, 1.0f});
	if (portalRenderTextureB_->IsReady()) {
		portalB_->SetTextureIndex(portalRenderTextureB_->GetSrvIndex());
	}

	activePointLightCount_ = 2;
	pointLights_[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
	pointLights_[0].position = {0.0f, 5.0f, 0.0f};
	pointLights_[0].intensity = 1.0f;
	pointLights_[0].radius = 10.0f;
	pointLights_[0].decay = 1.0f;
	pointLights_[1].color = {1.0f, 0.0f, 0.0f, 1.0f};
	pointLights_[1].position = {5.0f, 5.0f, 5.0f};
	pointLights_[1].intensity = 1.0f;
	pointLights_[1].radius = 10.0f;
	pointLights_[1].decay = 1.0f;

	directionalLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	directionalLight_.direction = {0.0f, -1.0f, 0.0f};
	directionalLight_.intensity = 1.0f;

	activeSpotLightCount_ = 2;
	spotLights_[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
	spotLights_[0].position = {2.0f, 1.25f, 0.0f};
	spotLights_[0].direction = {-1.0f, -1.0f, 0.0f};
	spotLights_[0].intensity = 4.0f;
	spotLights_[0].distance = 7.0f;
	spotLights_[0].decay = 2.0f;
	spotLights_[0].cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
	spotLights_[0].cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);

	spotLights_[1].color = {1.0f, 1.0f, 1.0f, 1.0f};
	spotLights_[1].position = {2.0f, 1.25f, 0.0f};
	spotLights_[1].direction = {-1.0f, -1.0f, 0.0f};
	spotLights_[1].intensity = 4.0f;
	spotLights_[1].distance = 7.0f;
	spotLights_[1].decay = 2.0f;
	spotLights_[1].cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
	spotLights_[1].cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);

	activeAreaLightCount_ = 2;
	areaLights_[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
	areaLights_[0].position = {0.0f, 3.0f, 0.0f};
	areaLights_[0].normal = {1.0f, -1.0f, 0.0f};
	areaLights_[0].intensity = 4.0f;
	areaLights_[0].width = 2.0f;
	areaLights_[0].height = 2.0f;
	areaLights_[0].radius = 0.1f;
	areaLights_[0].decay = 2.0f;

	areaLights_[1].color = {1.0f, 1.0f, 1.0f, 1.0f};
	areaLights_[1].position = {-5.0f, 3.0f, 0.0f};
	areaLights_[1].normal = {1.0f, -1.0f, 0.0f};
	areaLights_[1].intensity = 4.0f;
	areaLights_[1].width = 2.0f;
	areaLights_[1].height = 2.0f;
	areaLights_[1].radius = 0.1f;
	areaLights_[1].decay = 2.0f;
}

void SampleScene::Update() {
#ifdef USE_IMGUI
	if (ImGui::Begin("SampleCamera")) {
		ImGui::Checkbox("Use Debug Camera (F1)", &useDebugCamera_);
		ImGui::Text("Debug: LMB drag rotate, Shift+LMB drag pan, Wheel zoom");
		if (ImGui::TreeNode("Transform")) {

			if (!useDebugCamera_) {
				ImGui::DragFloat3("Scale", &cameraTransform_.scale.x, 0.01f);
				ImGui::DragFloat3("Rotate", &cameraTransform_.rotate.x, 0.01f);
				ImGui::DragFloat3("Translate", &cameraTransform_.translate.x, 0.01f);
			}
			ImGui::TreePop();
		}
		ImGui::End();
	}
	if (ImGui::Begin("Pad Input")) {
		ImGui::Text("押されているパッドボタン");
		const std::array<std::pair<Input::PadButton, const char*>, 14> padButtons = {
		    {{Input::PadButton::kButtonA, "A"},
		     {Input::PadButton::kButtonB, "B"},
		     {Input::PadButton::kButtonX, "X"},
		     {Input::PadButton::kButtonY, "Y"},
		     {Input::PadButton::kButtonLeftShoulder, "LB"},
		     {Input::PadButton::kButtonRightShoulder, "RB"},
		     {Input::PadButton::kButtonBack, "Back"},
		     {Input::PadButton::kButtonStart, "Start"},
		     {Input::PadButton::kButtonLeftThumb, "LStick"},
		     {Input::PadButton::kButtonRightThumb, "RStick"},
		     {Input::PadButton::kButtonUp, "DPad Up"},
		     {Input::PadButton::kButtonDown, "DPad Down"},
		     {Input::PadButton::kButtonLeft, "DPad Left"},
		     {Input::PadButton::kButtonRight, "DPad Right"}}
        };

		Input* input = Input::GetInstance();
		bool hasPush = false;
		for (const auto& [button, buttonName] : padButtons) {
			if (input->PushButton(button)) {
				ImGui::BulletText("%s", buttonName);
				hasPush = true;
			}
		}
		if (input->PushLeftTrigger()) {
			ImGui::BulletText("LT");
			hasPush = true;
		}
		if (input->PushRightTrigger()) {
			ImGui::BulletText("RT");
			hasPush = true;
		}
		if (!hasPush) {
			ImGui::TextDisabled("(なし)");
		}

		ImGui::Separator();
		ImGui::Text("このフレームで押されたボタン");
		bool hasTrigger = false;
		for (const auto& [button, buttonName] : padButtons) {
			if (input->TriggerButton(button)) {
				ImGui::BulletText("%s", buttonName);
				hasTrigger = true;
			}
		}
		if (input->TriggerLeftTrigger()) {
			ImGui::BulletText("LT");
			hasTrigger = true;
		}
		if (input->TriggerRightTrigger()) {
			ImGui::BulletText("RT");
			hasTrigger = true;
		}
		if (!hasTrigger) {
			ImGui::TextDisabled("(なし)");
		}
		ImGui::Separator();
		ImGui::Text("アナログ入力");
		const Vector2 leftStick = input->GetJoyStickLXY();
		const Vector2 rightStick = input->GetJoyStickRXY();
		ImGui::Text("Left Stick  : X %.3f / Y %.3f", leftStick.x, leftStick.y);
		ImGui::Text("Right Stick : X %.3f / Y %.3f", rightStick.x, rightStick.y);
		ImGui::Text("LT: %.3f", input->GetLeftTrigger());
		ImGui::Text("RT: %.3f", input->GetRightTrigger());
	}
	ImGui::End();
	if (ImGui::Begin("Human")) {
		if (ImGui::TreeNode("Transform")) {
			ImGui::DragFloat3("Scale", &humanTransform_.scale.x, 0.1f);
			ImGui::DragFloat3("Rotate", &humanTransform_.rotate.x, 0.1f);
			ImGui::DragFloat3("Translate", &humanTransform_.translate.x, 0.1f);
			ImGui::TreePop();
		}
		if (!humanAnimationClips_.empty()) {
			std::vector<const char*> animationNames;
			animationNames.reserve(humanAnimationClips_.size());
			for (const auto& clip : humanAnimationClips_) {
				animationNames.push_back(clip.name.c_str());
			}
			int selectedIndex = static_cast<int>(currentHumanAnimationIndex_);
			if (ImGui::Combo("Animation", &selectedIndex, animationNames.data(), static_cast<int>(animationNames.size()))) {
				currentHumanAnimationIndex_ = static_cast<size_t>(selectedIndex);
				humanObj_->SetAnimation(&humanAnimationClips_[currentHumanAnimationIndex_], true);
				humanAnimationTime_ = 0.0f;
			}
		}
	}
	ImGui::End();
	if (ImGui::Begin("Particle Editor")) {
		ImGui::Text("Sample Particle Emitter");
		if (ImGui::TreeNode("Emitter Transform")) {
			ImGui::DragFloat3("Scale##Particle", &particleTransform_.scale.x, 0.01f);
			ImGui::DragFloat3("Rotate##Particle", &particleTransform_.rotate.x, 0.01f);
			ImGui::DragFloat3("Translate##Particle", &particleTransform_.translate.x, 0.01f);
			ImGui::TreePop();
		}

		if (sampleParticleEmitter_) {
			float frequency = sampleParticleEmitter_->GetFrequency();
			if (ImGui::DragFloat("Frequency", &frequency, 0.01f, 0.0f, 10.0f)) {
				sampleParticleEmitter_->SetFrequency(frequency);
			}

			int emitCount = static_cast<int>(sampleParticleEmitter_->GetCount());
			if (ImGui::DragInt("EmitCount", &emitCount, 1, 1, 1024)) {
				sampleParticleEmitter_->SetCount(static_cast<uint32_t>(emitCount));
			}

			Vector3 acceleration = sampleParticleEmitter_->GetAcceleration();
			if (ImGui::DragFloat3("Acceleration", &acceleration.x, 0.01f)) {
				sampleParticleEmitter_->SetAcceleration(acceleration);
			}

			Vector3 areaMin = sampleParticleEmitter_->GetAreaMin();
			if (ImGui::DragFloat3("AreaMin", &areaMin.x, 0.01f)) {
				sampleParticleEmitter_->SetAreaMin(areaMin);
			}

			Vector3 areaMax = sampleParticleEmitter_->GetAreaMax();
			if (ImGui::DragFloat3("AreaMax", &areaMax.x, 0.01f)) {
				sampleParticleEmitter_->SetAreaMax(areaMax);
			}

			float life = sampleParticleEmitter_->GetLife();
			if (ImGui::DragFloat("Life", &life, 1.0f, 0.0f, 10000.0f)) {
				sampleParticleEmitter_->SetLife(life);
			}

			if (ImGui::Button("Emit Now")) {
				sampleParticleEmitter_->Emit();
			}
		}
	}
	ImGui::End();
	if (ImGui::Begin("ScreenEffectd")) {
		static const char* noiseBlendModes[] = {"Overwrite", "Add", "Subtract", "Multiply", "Screen"};
		ImGui::Checkbox("Fullscreen Grayscale (BT709)", &fullScreenGrayscaleEnabled_);
		ImGui::Checkbox("Fullscreen Sepia", &fullScreenSepiaEnabled_);
		ImGui::SliderFloat("Vignette Strength", &vignetteStrength_, 0.0f, 1.0f);
		ImGui::Checkbox("Random Noise (Monochrome)", &randomNoiseEnabled_);
		ImGui::SliderFloat("Random Noise Scale", &randomNoiseScale_, 1.0f, 4096.0f);
		ImGui::Combo("Random Noise Blend", &randomNoiseBlendMode_, noiseBlendModes, IM_ARRAYSIZE(noiseBlendModes));
	}
	ImGui::End();

#endif // USE_IMGUI
	if (useDebugCamera_) {
		debugCamera_->Update();
		camera_->SetViewProjectionMatrix(debugCamera_->GetViewMatrix(), debugCamera_->GetProjectionMatrix());
	} else {
		camera_->SetTransform(cameraTransform_);
		camera_->Update();
	}
	ParticleManager::GetInstance()->Update(camera_.get());
	if (sampleParticleEmitter_) {
		sampleParticleEmitter_->Update(particleTransform_);
	}
	Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight_);
	Object3dCommon::GetInstance()->SetPointLights(pointLights_.data(), activePointLightCount_);
	Object3dCommon::GetInstance()->SetSpotLights(spotLights_.data(), activeSpotLightCount_);
	Object3dCommon::GetInstance()->SetAreaLights(areaLights_.data(), activeAreaLightCount_);
	Object3dCommon::GetInstance()->SetFullScreenGrayscaleEnabled(fullScreenGrayscaleEnabled_);
	Object3dCommon::GetInstance()->SetFullScreenSepiaEnabled(fullScreenSepiaEnabled_);
	Object3dCommon::GetInstance()->GetDxCommon()->SetVignetteStrength(vignetteStrength_);
	Object3dCommon::GetInstance()->SetVignetteStrength(vignetteStrength_);
	Object3dCommon::GetInstance()->SetRandomNoiseEnabled(randomNoiseEnabled_);
	Object3dCommon::GetInstance()->SetRandomNoiseScale(randomNoiseScale_);
	Object3dCommon::GetInstance()->SetRandomNoiseBlendMode(randomNoiseBlendMode_);

	/*uvBallObj_->SetTransform(uvBallTransform_);*/
	/*planeGltf_->SetTransform(planeGTransform_);*/
	/*animatedCubeObj_->SetTransform(animatedCubeTransform_);*/
	/*humanObj_->SetTransform(humanTransform_);*/
	/*ringPrimitive_->SetTransform(ringTransform_);*/
	/*ringPrimitive_->SetColor({1.0f, 0.85f, 0.2f, 1.0f});*/
	portalA_->Update();
	portalB_->Update();
	portalCameraFromA_->SetRotate(portalATransform_.rotate);
	portalCameraFromA_->SetTranslate(portalATransform_.translate);
	portalCameraFromA_->Update();
	portalCameraFromB_->SetRotate(portalBTransform_.rotate);
	portalCameraFromB_->SetTranslate(portalBTransform_.translate);
	portalCameraFromB_->Update();
	portalRingA_->SetUvTransform(Vector3(1, 1, 1), Vector3(0, 0, ringUvRotation_), Vector3(0, 0, 0), Vector2(0.5f, 0.5f));
	portalRingB_->SetUvTransform(Vector3(1, 1, 1), Vector3(0, 0, -ringUvRotation_), Vector3(0, 0, 0), Vector2(0.5f, 0.5f));
	portalRingA_->Update();
	portalRingB_->Update();
	spherePrimitive_->Update();
	uvBallObj_->Update();
	fieldObj_->Update();
	planeGltf_->Update();
	animatedCubeObj_->Update();
	humanObj_->Update();
	ringUvRotation_ -= 0.05f;

	uvSprite->Update();

	Object3dCommon::GetInstance()->SetDefaultCamera(camera_.get());

	float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
	if (humanSkeleton_ && !humanAnimationClips_.empty()) {
		const Animation::AnimationData& currentAnimation = humanAnimationClips_[currentHumanAnimationIndex_];
		humanAnimationTime_ = Animation::AdvanceTime(currentAnimation, humanAnimationTime_, deltaTime, true);
		humanSkeleton_->ApplyAnimation(currentAnimation, humanAnimationTime_);
		humanSkeleton_->Update();
		if (!humanSkinCluster_.mappedPalette.empty()) {
			UpdateSkinCluster(humanSkinCluster_, *humanSkeleton_);
		}
		Matrix4x4 humanWorld = humanObj_->GetWorldMatrix();
		humanSkeleton_->SetObjectMatrix(humanWorld);
	}
}

void SampleScene::UpdatePortalCamera(const Transform& sourcePortal, const Transform& destinationPortal, Camera* outCamera) {
	if (!outCamera) {
		return;
	}
	const Matrix4x4 mainCameraWorld = camera_->GetWorldMatrix();
	const Matrix4x4 sourcePortalWorld = Function::MakeAffineMatrix(sourcePortal.scale, sourcePortal.rotate, sourcePortal.translate);
	const Matrix4x4 destinationPortalWorld = Function::MakeAffineMatrix(destinationPortal.scale, destinationPortal.rotate, destinationPortal.translate);
	const Matrix4x4 portalViewWorld = Function::Multiply(Function::Multiply(mainCameraWorld, Function::Inverse(sourcePortalWorld)), destinationPortalWorld);
	const Matrix4x4 portalViewMatrix = Function::Inverse(portalViewWorld);
	outCamera->SetViewProjectionMatrix(portalViewMatrix, camera_->GetProjectionMatrix());
}

void SampleScene::SetSceneCameraForDraw(Camera* camera) {
	uvBallObj_->SetCamera(camera);
	fieldObj_->SetCamera(camera);
	planeGltf_->SetCamera(camera);
	animatedCubeObj_->SetCamera(camera);
	humanObj_->SetCamera(camera);
	spherePrimitive_->SetCamera(camera);
	portalA_->SetCamera(camera);
	portalB_->SetCamera(camera);
	portalRingA_->SetCamera(camera);
	portalRingB_->SetCamera(camera);
}
void SampleScene::DrawSceneGeometry(bool includePortalA, bool includePortalB) {
	Object3dCommon::GetInstance()->DrawCommon();
	uvBallObj_->Draw();
	planeGltf_->Draw();
	fieldObj_->Draw();
	animatedCubeObj_->Draw();
	if (includePortalA) {
		portalA_->Draw();
	}
	if (includePortalB) {
		portalB_->Draw();
	}
	spherePrimitive_->Draw();
	Object3dCommon::GetInstance()->DrawCommonNoCullDepth();
	portalRingA_->Draw();
	portalRingB_->Draw();
	// if (sampleParticleEmitter_) {
	//	Object3dCommon::GetInstance()->DrawCommonNoCullDepth();
	//	sampleParticleEmitter_->Draw();
	// }
	Object3dCommon::GetInstance()->DrawCommonSkinningToon();
	humanObj_->Draw();
	Object3dCommon::GetInstance()->DrawCommonWireframeNoDepth();
}

void SampleScene::Draw() {
	Object3dCommon::GetInstance()->BeginShadowMapPass();
	Object3dCommon::GetInstance()->DrawCommonShadow();
	uvBallObj_->Draw();
	planeGltf_->Draw();
	fieldObj_->Draw();
	animatedCubeObj_->Draw();
	portalRingA_->Draw();
	portalRingB_->Draw();
	Object3dCommon::GetInstance()->EndShadowMapPass();

	auto* dxCommon = Object3dCommon::GetInstance()->GetDxCommon();
	auto* commandList = dxCommon->GetCommandList();
	if (portalRenderTextureA_ && portalRenderTextureA_->IsReady()) {
		portalRenderTextureA_->BeginRender(commandList);
		Object3dCommon::GetInstance()->SetDefaultCamera(portalCameraFromA_.get());
		SetSceneCameraForDraw(portalCameraFromA_.get());
		DrawSceneGeometry(false, true);
		portalRenderTextureA_->TransitionToShaderResource(commandList);
	}
	if (portalRenderTextureB_ && portalRenderTextureB_->IsReady()) {
		portalRenderTextureB_->BeginRender(commandList);
		Object3dCommon::GetInstance()->SetDefaultCamera(portalCameraFromB_.get());
		SetSceneCameraForDraw(portalCameraFromB_.get());
		DrawSceneGeometry(true, false);
		portalRenderTextureB_->TransitionToShaderResource(commandList);
	}

	dxCommon->SetMainRenderTarget();
	Object3dCommon::GetInstance()->SetDefaultCamera(camera_.get());
	SetSceneCameraForDraw(camera_.get());
	DrawSceneGeometry(true, true);
	SpriteCommon::GetInstance()->DrawCommon();
	uvSprite->Draw();
}

void SampleScene::Finalize() {}