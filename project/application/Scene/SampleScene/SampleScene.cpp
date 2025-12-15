#include "SampleScene.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include "Object3dCommon.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI


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

	
	uvBallObj_->Initialize(GameBase::GetInstance()->GetObject3dCommon()); 
	uvBallObj_->SetCamera(camera_.get());
	uvBallObj_->SetModel("uvBall");
	fieldObj_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	fieldObj_->SetCamera(camera_.get());
	fieldObj_->SetModel("terrain");

	uvBallTransform_ = {
		.scale{1.0f, 1.0f, 1.0f  },
		.rotate{0.0f, 0.0f, 0.0f  },
		.translate{0.0f, 0.0f, 0.0f}
    };
	uvBallObj_->SetTransform(uvBallTransform_);
	pointLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	pointLight_.position = {0.0f, 5.0f, 0.0f};
	pointLight_.intensity = 1.0f;
	pointLight_.radius = 10.0f;	
	pointLight_.decay = 1.0f;

	directionalLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	directionalLight_.direction = {0.0f, -1.0f, 0.0f};
	directionalLight_.intensity = 1.0f;
}

void SampleScene::Update() { 
#ifdef USE_IMGUI

	if (ImGui::Begin("SampleLight")) {
		if (ImGui::TreeNode("DirectionalLight")) {
		ImGui::ColorEdit4("LightColor", &directionalLight_.color.x);
		ImGui::DragFloat3("LightDirection", &directionalLight_.direction.x, 0.1f, -1.0f, 1.0f);
		ImGui::DragFloat("LightIntensity", &directionalLight_.intensity, 0.1f, 0.0f, 10.0f);
		ImGui::TreePop();
		}
		if (ImGui::TreeNode("PointLight")) {
			ImGui::ColorEdit4("PointLightColor", &pointLight_.color.x);
			ImGui::DragFloat("PointLightIntensity", &pointLight_.intensity,0.1f);
			ImGui::DragFloat3("PointLightPosition", &pointLight_.position.x,0.1f);
			ImGui::DragFloat("PointLightRadius", &pointLight_.radius,0.1f);
			ImGui::DragFloat("PointLightDecay", &pointLight_.decay,0.1f);
			ImGui::TreePop();
		}
		ImGui::End();
	}
	GameBase::GetInstance()->GetObject3dCommon()->SetDirectionalLight(directionalLight_);
	GameBase::GetInstance()->GetObject3dCommon()->SetPointLight(pointLight_);
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
	
		ImGui::End();
	}


#endif // USE_IMGUI
	camera_->Update();
	
	uvBallObj_->SetTransform(uvBallTransform_);
	uvBallObj_->Update();
	fieldObj_->Update();
}
void SampleScene::Draw() { 
	GameBase::GetInstance()->ModelCommonSet();
	uvBallObj_->Draw();
	fieldObj_->Draw();
}
void SampleScene::Finalize() {



}