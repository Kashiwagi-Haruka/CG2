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
#include <numbers>
#include <utility>
#include "SceneManager.h"
SampleScene::SampleScene() {

	uvBallObj_ = std::make_unique<Object3d>();
	fieldObj_ = std::make_unique<Object3d>();
	planeGltf_ = std::make_unique<Object3d>();
	animatedCubeObj_ = std::make_unique<Object3d>();
	humanObj_ = std::make_unique<Object3d>();
	cameraTransform_ = {
	    .scale{1.0f, 1.0f, 1.0f  },
        .rotate{0.0f, 0.0f, 0.0f  },
        .translate{0.0f, 5.0f, -10.0f}
    };

	camera_ = std::make_unique<Camera>();
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
        .translate{0.0f,                              1.0f,                      -3.0f  }
    };
	particleTransform_ = {
	    .scale{1.0f, 1.0f, 1.0f },
        .rotate{0.0f, 0.0f, 0.0f },
        .translate{0.0f, 1.0f, -3.0f}
    };
	sampleParticleEmitter_ = std::make_unique<ParticleEmitter>("sample", particleTransform_, 0.1f, 5, Vector3{0.0f, 0.0f, 0.0f}, Vector3{-0.5f, -0.5f, -0.5f}, Vector3{0.5f, 0.5f, 0.5f});
	uvBallObj_->SetTransform(uvBallTransform_);
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
	if (ImGui::Begin("SampleLight")) {
		if (ImGui::TreeNode("DirectionalLight")) {
			ImGui::ColorEdit4("LightColor", &directionalLight_.color.x);
			ImGui::DragFloat3("LightDirection", &directionalLight_.direction.x, 0.1f, -1.0f, 1.0f);
			ImGui::DragFloat("LightIntensity", &directionalLight_.intensity, 0.1f, 0.0f, 10.0f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("PointLight")) {
			ImGui::ColorEdit4("PointLightColor", &pointLights_[0].color.x);
			ImGui::DragFloat("PointLightIntensity", &pointLights_[0].intensity, 0.1f);
			ImGui::DragFloat3("PointLightPosition", &pointLights_[0].position.x, 0.1f);
			ImGui::DragFloat("PointLightRadius", &pointLights_[0].radius, 0.1f);
			ImGui::DragFloat("PointLightDecay", &pointLights_[0].decay, 0.1f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("PointLight1")) {
			ImGui::ColorEdit4("PointLightColor1", &pointLights_[1].color.x);
			ImGui::DragFloat("PointLightIntensity1", &pointLights_[1].intensity, 0.1f);
			ImGui::DragFloat3("PointLightPosition1", &pointLights_[1].position.x, 0.1f);
			ImGui::DragFloat("PointLightRadius1", &pointLights_[1].radius, 0.1f);
			ImGui::DragFloat("PointLightDecay1", &pointLights_[1].decay, 0.1f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("SpotLight")) {
			ImGui::ColorEdit4("SpotLightColor", &spotLights_[0].color.x);
			ImGui::DragFloat("SpotLightIntensity", &spotLights_[0].intensity, 0.1f);
			ImGui::DragFloat3("SpotLightPosition", &spotLights_[0].position.x, 0.1f);
			ImGui::DragFloat3("SpotLightDirection", &spotLights_[0].direction.x, 0.1f);
			ImGui::DragFloat("SpotLightDistance", &spotLights_[0].distance, 0.1f);
			ImGui::DragFloat("SpotLightDecay", &spotLights_[0].decay, 0.1f);
			ImGui::DragFloat("SpotLightCosAngle", &spotLights_[0].cosAngle, 0.1f, 0.0f, 1.0f);
			ImGui::DragFloat("SpotLightCosFalloffStart", &spotLights_[0].cosFalloffStart, 0.1f, 0.0f, 1.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("SpotLight1")) {
			ImGui::ColorEdit4("SpotLightColor1", &spotLights_[1].color.x);
			ImGui::DragFloat("SpotLightIntensity1", &spotLights_[1].intensity, 0.1f);
			ImGui::DragFloat3("SpotLightPosition1", &spotLights_[1].position.x, 0.1f);
			ImGui::DragFloat3("SpotLightDirection1", &spotLights_[1].direction.x, 0.1f);
			ImGui::DragFloat("SpotLightDistance1", &spotLights_[1].distance, 0.1f);
			ImGui::DragFloat("SpotLightDecay1", &spotLights_[1].decay, 0.1f);
			ImGui::DragFloat("SpotLightCosAngle1", &spotLights_[1].cosAngle, 0.1f, 0.0f, 1.0f);
			ImGui::DragFloat("SpotLightCosFalloffStart1", &spotLights_[1].cosFalloffStart, 0.1f, 0.0f, 1.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("AreaLight")) {
			ImGui::ColorEdit4("AreaLightColor", &areaLights_[0].color.x);
			ImGui::DragFloat("AreaLightIntensity", &areaLights_[0].intensity, 0.1f);
			ImGui::DragFloat3("AreaLightPosition", &areaLights_[0].position.x, 0.1f);
			ImGui::DragFloat3("AreaLightNormal", &areaLights_[0].normal.x, 0.1f);
			ImGui::DragFloat("AreaLightWidth", &areaLights_[0].width, 0.1f);
			ImGui::DragFloat("AreaLightHeight", &areaLights_[0].height, 0.1f);
			ImGui::DragFloat("AreaLightRadius", &areaLights_[0].radius, 0.1f);
			ImGui::DragFloat("AreaLightDecay", &areaLights_[0].decay, 0.1f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("AreaLight1")) {
			ImGui::ColorEdit4("AreaLightColor1", &areaLights_[1].color.x);
			ImGui::DragFloat("AreaLightIntensity1", &areaLights_[1].intensity, 0.1f);
			ImGui::DragFloat3("AreaLightPosition1", &areaLights_[1].position.x, 0.1f);
			ImGui::DragFloat3("AreaLightNormal1", &areaLights_[1].normal.x, 0.1f);
			ImGui::DragFloat("AreaLightWidth1", &areaLights_[1].width, 0.1f);
			ImGui::DragFloat("AreaLightHeight1", &areaLights_[1].height, 0.1f);
			ImGui::DragFloat("AreaLightRadius1", &areaLights_[1].radius, 0.1f);
			ImGui::DragFloat("AreaLightDecay1", &areaLights_[1].decay, 0.1f);
			ImGui::TreePop();
		}
	}
	ImGui::End();
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
	if (ImGui::Begin("SampleuvBall")) {
		if (ImGui::TreeNode("Transform")) {

			ImGui::DragFloat3("Scale", &uvBallTransform_.scale.x, 0.1f);
			ImGui::DragFloat3("Rotate", &uvBallTransform_.rotate.x, 0.1f);
			ImGui::DragFloat3("Translate", &uvBallTransform_.translate.x, 0.1f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Material")) {
			ImGui::ColorEdit4("MaterialColor", &color.x);
			ImGui::Checkbox("EnableLighting", &enableLighting);
			ImGui::DragFloat("Shininess", &shininess, 0.1f, 0.0f, 100.0f);
			ImGui::DragFloat("EnvironmentCoefficient", &environmentCoefficient, 0.01f, 0.0f, 1.0f);
			ImGui::Checkbox("Grayscale (BT709)", &grayscaleEnabled);
			ImGui::Checkbox("Sepia", &sepiaEnabled);
			uvBallObj_->SetColor(color);
			uvBallObj_->SetEnableLighting(enableLighting);
			uvBallObj_->SetShininess(shininess);
			uvBallObj_->SetEnvironmentCoefficient(environmentCoefficient);
			uvBallObj_->SetGrayscaleEnabled(grayscaleEnabled);
			uvBallObj_->SetSepiaEnabled(sepiaEnabled);
			ImGui::TreePop();
		}
	}
	ImGui::End();
	if (ImGui::Begin("planeG")) {
		if (ImGui::TreeNode("TransformG")) {

			ImGui::DragFloat3("ScaleG", &planeGTransform_.scale.x, 0.1f);
			ImGui::DragFloat3("RotateG", &planeGTransform_.rotate.x, 0.1f);
			ImGui::DragFloat3("TranslateG", &planeGTransform_.translate.x, 0.1f);
			ImGui::TreePop();
		}
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
	if (ImGui::Begin("Scene")) {
	
		if (ImGui::Button("Title")) {
			SceneManager::GetInstance()->ChangeScene("Title");
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

	uvBallObj_->SetTransform(uvBallTransform_);
	planeGltf_->SetTransform(planeGTransform_);
	animatedCubeObj_->SetTransform(animatedCubeTransform_);
	animatedCubeObj_->SetTransform(animatedCubeTransform_);
	humanObj_->SetTransform(humanTransform_);
	uvBallObj_->Update();
	fieldObj_->Update();
	planeGltf_->Update();
	animatedCubeObj_->Update();
	humanObj_->Update();

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
void SampleScene::Draw() {
	Object3dCommon::GetInstance()->BeginShadowMapPass();
	Object3dCommon::GetInstance()->DrawCommonShadow();
	uvBallObj_->Draw();
	planeGltf_->Draw();
	fieldObj_->Draw();
	/*animatedCubeObj_->Draw();*/
	Object3dCommon::GetInstance()->EndShadowMapPass();
	Object3dCommon::GetInstance()->GetDxCommon()->SetMainRenderTarget();
	Object3dCommon::GetInstance()->DrawCommon();
	 uvBallObj_->Draw();
	 planeGltf_->Draw();
	 fieldObj_->Draw();
	/*animatedCubeObj_->Draw();*/
	 //if (sampleParticleEmitter_) {
		//Object3dCommon::GetInstance()->DrawCommonNoCullDepth();
		// sampleParticleEmitter_->Draw();
	 //}
	/*Object3dCommon::GetInstance()->DrawCommonSkinningToon();
	humanObj_->Draw();*/
	//Object3dCommon::GetInstance()->DrawCommonWireframeNoDepth();
	//if (humanSkeleton_) {
	//	humanSkeleton_->DrawBones(camera_.get(), {0.2f, 0.6f, 1.0f, 1.0f}, {0.1f, 0.3f, 0.9f, 1.0f});
	//}
}
void SampleScene::Finalize() {}