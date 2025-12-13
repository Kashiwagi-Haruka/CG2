#include "SampleScene.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include "Object3dCommon.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI


SampleScene::SampleScene() {
	
	uvBallObj_ = std::make_unique<Object3d>();
	cameraTransform_ = {
	    .scale{1.0f, 1.0f, 1.0f  },
        .rotate{0.0f, 0.0f, 0.0f  },
        .translate{0.0f, 5.0f, -30.0f}
    };

	camera_ = std::make_unique<Camera>();
	camera_->SetTransform(cameraTransform_);
	
	ModelManeger::GetInstance()->LoadModel("uvBall");
}
void SampleScene::Initialize() {

	
	uvBallObj_->Initialize(GameBase::GetInstance()->GetObject3dCommon()); 
	uvBallObj_->SetCamera(camera_.get());
	uvBallObj_->SetModel("uvBall");
	uvBallObj_->SetScale({1.0f, 1.0f, 1.0f});
	uvBallObj_->SetRotate({0.0f, 0.0f, 0.0f});
	uvBallObj_->SetTranslate({0.0f, 0.0f, 0.0f});
	
	directionalLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	directionalLight_.direction = {0.0f, -1.0f, 0.0f};
	directionalLight_.intensity = 1.0f;
}

void SampleScene::Update() { 
#ifdef USE_IMGUI

	if (ImGui::Begin("SampleDirectionalLight")) {
		ImGui::ColorEdit4("LightColor", &directionalLight_.color.x);
		ImGui::DragFloat3("LightDirection", &directionalLight_.direction.x, 0.1f, -2.0f, 2.0f);
		ImGui::DragFloat("LightIntensity", &directionalLight_.intensity, 0.1f, 0.0f, 10.0f);
		GameBase::GetInstance()->GetObject3dCommon()->SetDirectionalLight(directionalLight_);
		ImGui::End();
	}
	if (ImGui::Begin("SampleMaterial")) {

		ImGui::ColorEdit4("MaterialColor", &color.x);
		ImGui::Checkbox("EnableLighting", &enableLighting);
		ImGui::DragFloat("Shininess", &shininess, 0.1f, 0.0f, 100.0f);
		uvBallObj_->SetColor(color);
		uvBallObj_->SetEnableLighting(enableLighting);
		uvBallObj_->SetShininess(shininess);
		ImGui::End();
	}


#endif // USE_IMGUI
	camera_->Update();
	uvBallObj_->Update();
}
void SampleScene::Draw() { 
	GameBase::GetInstance()->ModelCommonSet();
	uvBallObj_->Draw();

}
void SampleScene::Finalize() {



}