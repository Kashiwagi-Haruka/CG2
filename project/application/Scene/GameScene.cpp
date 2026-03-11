#include "GameScene.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include "ParticleManager.h"
#include "SceneManager.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include <algorithm>
#include <cmath>
#include <numbers>
#include "Engine/Texture/Data/Color.h"

GameScene::GameScene() {
	sceneTransition = std::make_unique<SceneTransition>();

	pause = std::make_unique<Pause>();
	BGMData = Audio::GetInstance()->SoundLoadFile("Resources/audio/BGM/Tailshaft.mp3");
	Audio::GetInstance()->SetSoundVolume(&BGMData, 0.3f);
}

GameScene::~GameScene() {}

void GameScene::Finalize() {

	Audio::GetInstance()->SoundUnload(&BGMData);

}

void GameScene::Initialize() {

	sceneEndClear = false;
	sceneEndOver = false;
	isBGMPlaying = false;

	sceneTransition->Initialize(false);
	isTransitionIn = true;
	isTransitionOut = false;
	nextSceneName.clear();
	pointLights_.ClearLights();
	pointLights_.AddPointLight("Point0");
	pointLights_.SetLightProperties("Point0", {1.0f, 1.0f, 1.0f, 1.0f}, {-75.0f, 10.0f, -75.0f}, 1.0f, 10.0f, 0.7f);
	pointLights_.AddPointLight("Point1");
	pointLights_.SetLightProperties("Point1", {1.0f, 0.9f, 0.9f, 1.0f}, {75.0f, 5.0f, 75.0f}, 0.0f, 10.0f, 0.7f);
	pointLights_.AddPointLight("Point2");
	pointLights_.SetLightProperties("Point2", {0.4f, 0.4f, 1.0f, 1.0f}, {-75.0f, 5.0f, 75.0f}, 1.0f, 5.0f, 0.7f);

	directionalLight_.SetColor(Color::RGBAToVector4(76, 96, 178, 255));
	directionalLight_.SetDirection({0.0f, -1.0f, 0.5f});
	directionalLight_.SetIntensity(1.0f);

	spotLights_.ClearSpotLights();
	spotLights_.AddSpotLight("Spot0");
	spotLights_.SetSpotLightProperties(
	    "Spot0", {1.0f, 1.0f, 1.0f, 1.0f}, {-50.0f, 5.0f, -50.0f}, 0.0f, {0.0f, 1.0f, 0.0f}, 7.0f, 2.0f, std::numbers::pi_v<float> / 3.0f, std::numbers::pi_v<float> / 4.0f);
	pause->Initialize();
}

void GameScene::DebugImGui() {

#ifdef USE_IMGUI
	if (ImGui::Begin("SampleLight")) {
		if (ImGui::TreeNode("DirectionalLight")) {
			Vector4 lightColor = directionalLight_.GetColor();
			if (ImGui::ColorEdit4("LightColor", &lightColor.x)) {
				directionalLight_.SetColor(lightColor);
			}
			Vector3 lightDirection = directionalLight_.GetDirection();
			if (ImGui::DragFloat3("LightDirection", &lightDirection.x, 0.1f, -1.0f, 1.0f)) {
				directionalLight_.SetDirection(lightDirection);
			}
			float lightIntensity = directionalLight_.GetIntensity();
			if (ImGui::DragFloat("LightIntensity", &lightIntensity, 0.1f, 0.0f, 10.0f)) {
				directionalLight_.SetIntensity(lightIntensity);
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("PointLight")) {

			for (int index = 0; index < pointLights_.GetLightCount(); ++index) {
				ImGui::PushID(static_cast<int>(index));
				if (ImGui::TreeNode("PointLight")) {
					const std::string pointName = "Point" + std::to_string(index);
					Vector4 pointColor = pointLights_.GetLightColor(pointName);
					if (ImGui::ColorEdit4("PointLightColor", &pointColor.x)) {
						pointLights_.SetLightColor(pointName, pointColor);
					}
					float pointIntensity = pointLights_.GetLightIntensity(pointName);
					if (ImGui::DragFloat("PointLightIntensity", &pointIntensity, 0.1f)) {
						pointLights_.SetLightIntensity(pointName, pointIntensity);
					}
					Vector3 pointPosition = pointLights_.GetLightPosition(pointName);
					if (ImGui::DragFloat3("PointLightPosition", &pointPosition.x, 0.1f)) {
						pointLights_.SetLightPosition(pointName, pointPosition);
					}
					float pointRadius = pointLights_.GetLightRadius(pointName);
					if (ImGui::DragFloat("PointLightRadius", &pointRadius, 0.1f)) {
						pointLights_.SetLightRadius(pointName, pointRadius);
					}
					float pointDecay = pointLights_.GetLightDecay(pointName);
					if (ImGui::DragFloat("PointLightDecay", &pointDecay, 0.1f)) {
						pointLights_.SetLightDecay(pointName, pointDecay);
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("SpotLight")) {
			Vector4 spotColor = spotLights_.GetSpotLightColor("Spot0");
			if (ImGui::ColorEdit4("SpotLightColor", &spotColor.x)) {
				spotLights_.SetSpotLightColor("Spot0", spotColor);
			}
			float spotIntensity = spotLights_.GetSpotLightIntensity("Spot0");
			if (ImGui::DragFloat("SpotLightIntensity", &spotIntensity, 0.1f)) {
				spotLights_.SetSpotLightIntensity("Spot0", spotIntensity);
			}
			Vector3 spotPosition = spotLights_.GetSpotLightPosition("Spot0");
			if (ImGui::DragFloat3("SpotLightPosition", &spotPosition.x, 0.1f)) {
				spotLights_.SetSpotLightPosition("Spot0", spotPosition);
			}
			Vector3 spotDirection = spotLights_.GetSpotLightDirection("Spot0");
			if (ImGui::DragFloat3("SpotLightDirection", &spotDirection.x, 0.1f)) {
				spotLights_.SetSpotLightDirection("Spot0", spotDirection);
			}
			float spotDistance = spotLights_.GetSpotLightDistance("Spot0");
			if (ImGui::DragFloat("SpotLightDistance", &spotDistance, 0.1f)) {
				spotLights_.SetSpotLightDistance("Spot0", spotDistance);
			}
			float spotDecay = spotLights_.GetSpotLightDecay("Spot0");
			if (ImGui::DragFloat("SpotLightDecay", &spotDecay, 0.1f)) {
				spotLights_.SetSpotLightDecay("Spot0", spotDecay);
			}
			float spotAngle = spotLights_.GetSpotLightAngle("Spot0");
			if (ImGui::DragFloat("SpotLightAngle", &spotAngle, 0.01f, 0.0f, std::numbers::pi_v<float>)) {
				spotLights_.SetSpotLightAngle("Spot0", spotAngle);
			}
			float spotFalloffStartAngle = spotLights_.GetSpotLightFalloffStartAngle("Spot0");
			if (ImGui::DragFloat("SpotLightFalloffStartAngle", &spotFalloffStartAngle, 0.01f, 0.0f, std::numbers::pi_v<float>)) {
				spotLights_.SetSpotLightFalloffStartAngle("Spot0", spotFalloffStartAngle);
			}
			ImGui::TreePop();
		}
	}
	ImGui::End();

#endif // USE_IMGUI
}

void GameScene::Update() {
	if (!isBGMPlaying) {
		Audio::GetInstance()->SoundPlayWave(BGMData, true);
		isBGMPlaying = true;
	}

	DebugImGui();
	pause->Update(isPause);
	Pause::Action pauseAction = pause->ConsumeAction();

	Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight_);
	Object3dCommon::GetInstance()->SetPointLights(pointLights_);
	Object3dCommon::GetInstance()->SetSpotLights(spotLights_);

	if (isTransitionIn || isTransitionOut) {
		sceneTransition->Update();
		if (sceneTransition->IsEnd() && isTransitionIn) {
			isTransitionIn = false;
		}
		if (sceneTransition->IsEnd() && isTransitionOut) {
			SceneManager::GetInstance()->ChangeScene(nextSceneName);
		}
	}
}

void GameScene::Draw() {
	
	pause->Draw();
	if (isTransitionIn || isTransitionOut) {

		sceneTransition->Draw();
	}
}