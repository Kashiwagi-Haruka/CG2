#include "PlayerModels.h"
#include <cmath>
#include "Function.h"
#include "ModelManeger.h"
#include "GameBase.h"
#include "imgui.h"
PlayerModels::PlayerModels() {
	ModelManeger::GetInstance()->LoadModel("playerRarm");
	ModelManeger::GetInstance()->LoadModel("playerbody");

	ModelManeger::GetInstance()->LoadModel("playerLarm");
	ModelManeger::GetInstance()->LoadModel("playerRleg");
	ModelManeger::GetInstance()->LoadModel("playerLleg");
};
PlayerModels::~PlayerModels() {};

void PlayerModels::Initialize() {



	head_ = std::make_unique<Object3d>();
	armR_ = std::make_unique<Object3d>();
	armL_ = std::make_unique<Object3d>();
	legR_ = std::make_unique<Object3d>();
	legL_ = std::make_unique<Object3d>();

	head_->SetModel("playerbody");
	armR_->SetModel("playerRarm");
	armL_->SetModel("playerLarm");
	legL_->SetModel("playerLleg");
	legR_->SetModel("playerRleg");

	head_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	armR_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	armL_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	legL_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	legR_->Initialize(GameBase::GetInstance()->GetObject3dCommon());

	// ★ ローカル位置（player基準）
	headT_ = {
	    .scale{1,    1,    1   },
        .rotate{0,    0,    0   },
        .translate = {0.0f, 0.35f, 0.0f}
    };

	armRT_ = {
	    .scale{1,    1,    1   },
        .rotate{0,    0,    0   },
        .translate = {0.4f, 1.2f, 0.0f}
    };
	armLT_={
	    .scale{1,1,1},
        .rotate{0,0,0},
        .translate{-0.4f, 1.2f, 0.0f}
    };

	legRT_ = {
	    .scale{1,1,1},
        .rotate{0,0,0},
        .translate{0.2f, 0.3f, 0.0f}
    };
	legLT_ = {
		.scale{1, 1, 1},
        .rotate{0,0,0},
		.translate{-0.2f, 0.3f, 0.0f}};
}

void PlayerModels::Update() {

	static float t = 0.0f;
	t += 0.1f;

	if (state_ == walk) {
		armRT_.rotate.x = std::sin(t) * 0.5f;
		armLT_.rotate.x = -std::sin(t) * 0.5f;

		legRT_.rotate.x = -std::sin(t) * 0.5f;
		legLT_.rotate.x = std::sin(t) * 0.5f;
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

	if (ImGui::CollapsingHeader("Body")) {
		ImGui::DragFloat3("Body Pos", &headT_.translate.x, 0.01f);
		ImGui::DragFloat3("Body Rot", &headT_.rotate.x, 0.01f);
		ImGui::DragFloat3("Body Scale", &headT_.scale.x, 0.01f);
	}

	if (ImGui::CollapsingHeader("Right Arm")) {
		ImGui::DragFloat3("RArm Pos", &armRT_.translate.x, 0.01f);
		ImGui::DragFloat3("RArm Rot", &armRT_.rotate.x, 0.01f);
		ImGui::DragFloat3("RArm Scale", &armRT_.scale.x, 0.01f);
	}

	if (ImGui::CollapsingHeader("Left Arm")) {
		ImGui::DragFloat3("LArm Pos", &armLT_.translate.x, 0.01f);
		ImGui::DragFloat3("LArm Rot", &armLT_.rotate.x, 0.01f);
		ImGui::DragFloat3("LArm Scale", &armLT_.scale.x, 0.01f);
	}

	if (ImGui::CollapsingHeader("Right Leg")) {
		ImGui::DragFloat3("RLeg Pos", &legRT_.translate.x, 0.01f);
		ImGui::DragFloat3("RLeg Rot", &legRT_.rotate.x, 0.01f);
		ImGui::DragFloat3("RLeg Scale", &legRT_.scale.x, 0.01f);
	}

	if (ImGui::CollapsingHeader("Left Leg")) {
		ImGui::DragFloat3("LLeg Pos", &legLT_.translate.x, 0.01f);
		ImGui::DragFloat3("LLeg Rot", &legLT_.rotate.x, 0.01f);
		ImGui::DragFloat3("LLeg Scale", &legLT_.scale.x, 0.01f);
	}

	ImGui::End();
	#endif
}

void PlayerModels::Draw() {

	Matrix4x4 playerWorld = Function::MakeAffineMatrix(player_.scale, player_.rotate, player_.translate);
	Matrix4x4 bodyM = Function::MakeAffineMatrix(headT_.scale, headT_.rotate, headT_.translate);
	Matrix4x4 armRM = Function::MakeAffineMatrix(armRT_.scale, armRT_.rotate, armRT_.translate);
	Matrix4x4 armLM = Function::MakeAffineMatrix(armLT_.scale, armLT_.rotate, armLT_.translate);
	Matrix4x4 legRM = Function::MakeAffineMatrix(legRT_.scale, legRT_.rotate, legRT_.translate);
	Matrix4x4 legLM = Function::MakeAffineMatrix(legLT_.scale, legLT_.rotate, legLT_.translate);


	// head
	head_->SetWorldMatrix(Function::Multiply(playerWorld, bodyM));
	head_->SetCamera(camera_);
	head_->Update();
	head_->Draw();

	// 右腕
	armR_->SetWorldMatrix(Function::Multiply(playerWorld, armRM));
	armR_->SetCamera(camera_);
	armR_->Update();
	armR_->Draw();

	// 左腕
	armL_->SetWorldMatrix(Function::Multiply(playerWorld, armLM));
	armL_->SetCamera(camera_);
	armL_->Update();
	armL_->Draw();

	// 右脚
	legR_->SetWorldMatrix(Function::Multiply(playerWorld, legRM));
	legR_->SetCamera(camera_);
	legR_->Update();
	legR_->Draw();

	// 左脚
	legL_->SetWorldMatrix(Function::Multiply(playerWorld, legLM));
	legL_->SetCamera(camera_);
	legL_->Update();
	legL_->Draw();
}
