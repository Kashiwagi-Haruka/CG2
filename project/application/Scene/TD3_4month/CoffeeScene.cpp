#include "CoffeeScene.h"
#include "Function.h"
#include "Input.h"
#include "Light/DirectionalLight.h"
#include "Object3d/Object3dCommon.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI
#include <numbers>

namespace {
constexpr float kRoomWidth = 9.0f;
constexpr float kRoomHeight = 2.4f;
constexpr float kRoomDepth = 15.0f;
} // namespace

CoffeeScene::CoffeeScene() {
	for (auto& wall : roomWalls_) {
		wall = std::make_unique<Primitive>();
	}
	camera_ = std::make_unique<Camera>();
	debugCamera_ = std::make_unique<DebugCamera>();
	cameraTransform_ = {
	    .scale = {1.0f, 1.0f, 1.0f},
	    .rotate = {0.0f, 0.0f, 0.0f},
	    .translate = {0.0f, 1.5f, 0.0f},
	};
}

void CoffeeScene::Initialize() {
	const float halfWidth = kRoomWidth * 0.5f;
	const float halfHeight = kRoomHeight * 0.5f;
	const float halfDepth = kRoomDepth * 0.5f;

	camera_->SetTransform(cameraTransform_);
	debugCamera_->Initialize();
	debugCamera_->SetTranslation(cameraTransform_.translate);

	Object3dCommon::GetInstance()->SetRandomNoiseEnabled(false);

	for (auto& wall : roomWalls_) {
		wall->Initialize(Primitive::Plane);
		wall->SetCamera(camera_.get());
		wall->SetEnableLighting(true);
		wall->SetColor({0.95f, 0.95f, 0.95f, 1.0f});
	}

	// 床 (+Y を向く)
	roomWalls_[1]->SetScale({kRoomWidth, kRoomDepth, 1.0f});
	roomWalls_[1]->SetRotate({std::numbers::pi_v<float> * 0.5f, 0.0f, 0.0f});
	roomWalls_[1]->SetTranslate({0.0f, -halfHeight, 0.0f});

	// 天井 (-Y を向く)
	roomWalls_[1]->SetScale({kRoomWidth, kRoomDepth, 1.0f});
	roomWalls_[1]->SetRotate({-std::numbers::pi_v<float> * 0.5f, 0.0f, 0.0f});
	roomWalls_[1]->SetTranslate({0.0f, halfHeight, 0.0f});

	// 手前壁 (-Z を向く)
	roomWalls_[2]->SetScale({kRoomWidth, kRoomHeight, 1.0f});
	roomWalls_[2]->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});
	roomWalls_[2]->SetTranslate({0.0f, 0.0f, halfDepth});

	// 奥壁 (+Z を向く)
	roomWalls_[3]->SetScale({kRoomWidth, kRoomHeight, 1.0f});
	roomWalls_[3]->SetRotate({0.0f, 0.0f, 0.0f});
	roomWalls_[3]->SetTranslate({0.0f, 0.0f, -halfDepth});

	// 左壁 (+X を向く)
	roomWalls_[4]->SetScale({kRoomDepth, kRoomHeight, 1.0f});
	roomWalls_[4]->SetRotate({0.0f, -std::numbers::pi_v<float> * 0.5f, 0.0f});
	roomWalls_[4]->SetTranslate({-halfWidth, 0.0f, 0.0f});

	// 右壁 (-X を向く)
	roomWalls_[5]->SetScale({kRoomDepth, kRoomHeight, 1.0f});
	roomWalls_[5]->SetRotate({0.0f, std::numbers::pi_v<float> * 0.5f, 0.0f});
	roomWalls_[5]->SetTranslate({halfWidth, 0.0f, 0.0f});
}

void CoffeeScene::Update() {
	auto* input = Input::GetInstance();
	if (input->TriggerKey(DIK_F1)) {
		useDebugCamera_ = !useDebugCamera_;
	}

#ifdef USE_IMGUI
	if (ImGui::Begin("CoffeeCamera")) {
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
	Object3dCommon::GetInstance()->SetDefaultCamera(camera_.get());
	for (auto& wall : roomWalls_) {
		wall->SetCamera(camera_.get());
		wall->Update();
	}

	DirectionalLight directionalLight{};
	directionalLight.color = {1.0f, 1.0f, 1.0f, 1.0f};
	directionalLight.direction = Function::Normalize({0.1f, -1.0f, -0.2f});
	directionalLight.intensity = 1.0f;
	Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight);
}

void CoffeeScene::Draw() {
	Object3dCommon::GetInstance()->DrawCommonNoCull();
	//for (auto& wall : roomWalls_) {
	//	wall->Draw();
	//}
	roomWalls_[0]->Draw();
}

void CoffeeScene::Finalize() {}