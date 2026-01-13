#include "SampleScene.h"
#include "GameBase.h"
#include "Model/ModelManeger.h"
#include "Object3d/Object3dCommon.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI
#include <numbers>

SampleScene::SampleScene() {

	uvBallObj_ = std::make_unique<Object3d>();
	fieldObj_ = std::make_unique<Object3d>();
	planeGltf_ = std::make_unique<Object3d>();
	cameraTransform_ = {
	    .scale{1.0f, 1.0f, 1.0f  },
        .rotate{0.0f, 0.0f, 0.0f  },
        .translate{0.0f, 5.0f, -30.0f}
    };

	camera_ = std::make_unique<Camera>();
	camera_->SetTransform(cameraTransform_);

	ModelManager::GetInstance()->LoadModel("uvBall");
	ModelManager::GetInstance()->LoadModel("terrain");
	ModelManager::GetInstance()->LoadGltfModel("planeG");
}
void SampleScene::Initialize() {

	uvBallObj_->Initialize();
	uvBallObj_->SetCamera(camera_.get());
	uvBallObj_->SetModel("uvBall");
	fieldObj_->Initialize();
	fieldObj_->SetCamera(camera_.get());
	fieldObj_->SetModel("terrain");
	planeGltf_->Initialize();
	planeGltf_->SetCamera(camera_.get());
	planeGltf_->SetModel("planeG");

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
	uvBallObj_->SetTransform(uvBallTransform_);
	planeGltf_->SetTransform(uvBallTransform_);
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

	activeSpotLightCount_ = 1;
	spotLights_[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
	spotLights_[0].position = {2.0f, 1.25f, 0.0f};
	spotLights_[0].direction = {-1.0f, -1.0f, 0.0f};
	spotLights_[0].intensity = 4.0f;
	spotLights_[0].distance = 7.0f;
	spotLights_[0].decay = 2.0f;
	spotLights_[0].cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
	spotLights_[0].cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);
}

void SampleScene::Update() {
#ifdef USE_IMGUI
	if (ImGui::Begin("SampleCamera")) {
		if (ImGui::TreeNode("Transform")) {

			ImGui::DragFloat3("Scale", &cameraTransform_.scale.x, 0.1f);
			ImGui::DragFloat3("Rotate", &cameraTransform_.rotate.x, 0.1f);
			ImGui::DragFloat3("Translate", &cameraTransform_.translate.x, 0.1f);
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
		if (ImGui::TreeNode("PointLigh1")) {
			ImGui::ColorEdit4("PointLightColo1", &pointLights_[1].color.x);
			ImGui::DragFloat("PointLightIntensit1", &pointLights_[1].intensity, 0.1f);
			ImGui::DragFloat3("PointLightPositio1", &pointLights_[1].position.x, 0.1f);
			ImGui::DragFloat("PointLightRadiu1", &pointLights_[1].radius, 0.1f);
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
			uvBallObj_->SetColor(color);
			uvBallObj_->SetEnableLighting(enableLighting);
			uvBallObj_->SetShininess(shininess);
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

#endif // USE_IMGUI
	camera_->SetTransform(cameraTransform_);
	Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight_);
	Object3dCommon::GetInstance()->SetPointLights(pointLights_.data(), activePointLightCount_);
	Object3dCommon::GetInstance()->SetSpotLights(spotLights_.data(), activeSpotLightCount_);
	camera_->Update();

	uvBallObj_->SetTransform(uvBallTransform_);
	planeGltf_->SetTransform(planeGTransform_);
	uvBallObj_->Update();
	fieldObj_->Update();
	planeGltf_->Update();
}
void SampleScene::Draw() {
	Object3dCommon::GetInstance()->DrawCommon();
	uvBallObj_->Draw();
	planeGltf_->Draw();
	fieldObj_->Draw();
}
void SampleScene::Finalize() {}
