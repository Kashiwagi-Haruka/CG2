#include "PlayerModels.h"
#include <cmath>
#include "Function.h"
#include "Model/ModelManager.h"
#include "GameBase.h"
#include "imgui.h"
#include <numbers>
PlayerModels::PlayerModels() {

};
PlayerModels::~PlayerModels() {};

void PlayerModels::Initialize() {

	Sizuku_ = std::make_unique<Object3d>();

	Sizuku_->SetModel("sizuku");

	Sizuku_->Initialize();

}

void PlayerModels::Update() {

	static float t = 0.0f;
	t += 0.1f;
	switch (state_) {
	case PlayerModels::idle:


		break;
	case PlayerModels::walk:
		
			
		
		break;
	case PlayerModels::attack1:
		
		break;
	case PlayerModels::attack2:
		
		break;
	case PlayerModels::attack3:
		
		break;
	case PlayerModels::attack4:
		
		break;
	case PlayerModels::fallingAttack:
		
		break;
	case PlayerModels::skillAttack:
		
		break;
	case PlayerModels::damage:
		
		break;
	default:
		break;
	}

	#ifdef USE_IMGUI
	if (!ImGui::Begin("Player Parts Adjust")) {
		ImGui::End();
		return;
	}

	if (ImGui::CollapsingHeader("Player Root")) {
		ImGui::DragFloat3("Player Pos", &player_.translate.x, 0.01f);
		ImGui::DragFloat3("Player Rot", &player_.rotate.x, 0.01f);
		ImGui::DragFloat3("Player Scale", &player_.scale.x, 0.01f);
	}



	ImGui::End();
	#endif
}

void PlayerModels::Draw() {

	Matrix4x4 playerWorld = Function::MakeAffineMatrix(player_.scale, player_.rotate, player_.translate);


	Sizuku_->SetWorldMatrix(playerWorld);
	Sizuku_->SetCamera(camera_);
	Sizuku_->Update();
	Sizuku_->Draw();

	
}
