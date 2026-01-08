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
	cameraTransform_ = {
	    .scale{1.0f, 1.0f, 1.0f  },
        .rotate{0.0f, 0.0f, 0.0f  },
        .translate{0.0f, 5.0f, -30.0f}
    };

	camera_ = std::make_unique<Camera>();
	camera_->SetTransform(cameraTransform_);
	
	ModelManeger::GetInstance()->LoadModel("uvBall");
	ModelManeger::GetInstance()->LoadModel("terrain");
}
void SampleScene::Initialize() {

	
	uvBallObj_->Initialize(); 
	uvBallObj_->SetCamera(camera_.get());
	uvBallObj_->SetModel("uvBall");
	fieldObj_->Initialize();
	fieldObj_->SetCamera(camera_.get());
	fieldObj_->SetModel("terrain");

	uvBallTransform_ = {
		.scale{1.0f, 1.0f, 1.0f  },
		.rotate{0.0f, 0.0f, 0.0f  },
		.translate{0.0f, 0.0f, 0.0f}
    };
	uvBallObj_->SetTransform(uvBallTransform_);
	activePointLightCount_ = 2;
	pointLights_[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
	pointLights_[0].position = {0.0f, 5.0f, 0.0f};
	pointLights_[0].intensity = 1.0f;
	pointLights_[0].radius = 10.0f;
	pointLights_[0].decay = 1.0f;
	pointLights_[1].color = {1.0f, 0.8f, 0.8f, 1.0f};
	pointLights_[1].position = {5.0f, 3.0f, -5.0f};
	pointLights_[1].intensity = 0.6f;
	pointLights_[1].radius = 12.0f;
	pointLights_[1].decay = 1.0f;

	directionalLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	directionalLight_.direction = {0.0f, -1.0f, 0.0f};
	directionalLight_.intensity = 1.0f;

	spotLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	spotLight_.position = {2.0f, 1.25f, 0.0f};
	spotLight_.direction = {-1.0f, -1.0f, 0.0f};
	spotLight_.intensity = 4.0f;
	spotLight_.distance = 7.0f;
	spotLight_.decay = 2.0f;
	spotLight_.cosAngle = std::cos(std::numbers::pi_v<float>/3.0f);
	spotLight_.cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);
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
			int lightCount = static_cast<int>(activePointLightCount_);
			if (ImGui::SliderInt("PointLightCount", &lightCount, 0, static_cast<int>(kMaxPointLights))) {
				activePointLightCount_ = static_cast<uint32_t>(lightCount);
			}
			for (uint32_t index = 0; index < activePointLightCount_; ++index) {
				ImGui::PushID(static_cast<int>(index));
				if (ImGui::TreeNode("PointLight")) {
					ImGui::ColorEdit4("PointLightColor", &pointLights_[index].color.x);
					ImGui::DragFloat("PointLightIntensity", &pointLights_[index].intensity, 0.1f);
					ImGui::DragFloat3("PointLightPosition", &pointLights_[index].position.x, 0.1f);
					ImGui::DragFloat("PointLightRadius", &pointLights_[index].radius, 0.1f);
					ImGui::DragFloat("PointLightDecay", &pointLights_[index].decay, 0.1f);
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("SpotLight")) {
			ImGui::ColorEdit4("SpotLightColor", &spotLight_.color.x);
			ImGui::DragFloat("SpotLightIntensity", &spotLight_.intensity, 0.1f);
			ImGui::DragFloat3("SpotLightPosition", &spotLight_.position.x, 0.1f);
			ImGui::DragFloat3("SpotLightDirection", &spotLight_.direction.x, 0.1f);
			ImGui::DragFloat("SpotLightDistance", &spotLight_.distance, 0.1f);
			ImGui::DragFloat("SpotLightDecay", &spotLight_.decay, 0.1f);
			ImGui::DragFloat("SpotLightCosAngle", &spotLight_.cosAngle, 0.1f, 0.0f, 1.0f);
			ImGui::DragFloat("SpotLightCosFalloffStart", &spotLight_.cosFalloffStart, 0.1f, 0.0f, 1.0f);
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

#endif // USE_IMGUI
	camera_->SetTransform(cameraTransform_);
	Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight_);
	Object3dCommon::GetInstance()->SetPointLights(pointLights_.data(), activePointLightCount_);
	Object3dCommon::GetInstance()->SetSpotLight(spotLight_);
	camera_->Update();

	uvBallObj_->SetTransform(uvBallTransform_);
	uvBallObj_->Update();
	fieldObj_->Update();
}
void SampleScene::Draw() {
	Object3dCommon::GetInstance()->DrawCommon();
	uvBallObj_->Draw();
	fieldObj_->Draw();
}
void SampleScene::Finalize() {}

