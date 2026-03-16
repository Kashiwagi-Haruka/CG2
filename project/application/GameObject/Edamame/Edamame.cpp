#include "Edamame.h"
#include <Model/ModelManager.h>
#include <GameObject/YoshidaMath/YoshidaMath.h>
#include <GameObject/KeyBindConfig.h>
#include <Function.h>
#include <GameObject/YoshidaMath/CollisionManager/Collider.h>
#include"Object3d/Object3dCommon.h"

Edamame::Edamame()
{
	obj_ = std::make_unique<Object3d>();
	// モデルをセット
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/edamame_billboard", "edamame_billboard");
	obj_->SetModel("edamame_billboard");
	//枝豆
	edamameBGM_ = Audio::GetInstance()->SoundLoadFile("Resources/TD3_3102/Audio/BGM/scl_k1_cb.mp3");
	Audio::GetInstance()->SetSoundVolume(&edamameBGM_, 1.0f);
	//枝豆知識
	edamameTrivia_ = std::make_unique<EdamameTrivia>();
}

Edamame::~Edamame()
{
	Audio::GetInstance()->SoundUnload(&edamameBGM_);
}

void Edamame::Initialize()
{
	worldTransform_ = {
		.scale{0.5f, 0.5f, 0.5f},
		.rotate{0.0f, 0.0f, 0.0f},
		.translate{-4.0f, 0.25f, 0.0f}
	};

	obj_->Initialize();
	localAABB_ = { .min = { -0.5f,-0.5f,-0.5f},.max = {0.5f,0.5f,0.5f} };

	isPlaySound_ = false;
	//枝豆知識
	edamameTrivia_->Initialize();

}

void Edamame::Update()
{
	obj_->SetTransform(worldTransform_);
	obj_->UpdateBillboard();

}

void Edamame::Draw()
{
	obj_->Draw();
}

void Edamame::SetPlayerCamera(PlayerCamera* camera)
{
	playerCamera_ = camera;
}

void Edamame::SetCamera(Camera* camera)
{
	obj_->SetCamera(camera);
	obj_->UpdateCameraMatrices();
}

void Edamame::SetModel(const std::string& filePath)
{
	obj_->SetModel(filePath);
}

void Edamame::CheckCollision()
{
	//keyとrayの当たり判定
	if (OnCollisionRay()) {
		if (PlayerCommand::GetInstance()->Interact()) {
			// カーソルに追従させて持ち上げる処理
			Vector3 origin = playerCamera_->GetTransform().translate;
			worldTransform_.translate = origin + (Function::Normalize(playerCamera_->GetRay().diff));
			worldTransform_.translate.y = (std::max)(worldTransform_.translate.y, 0.25f);
		}

		if (PlayerCommand::GetInstance()->InteractTrigger()) {
			if (!isPlaySound_) {
				isPlaySound_ = true;
				Audio::GetInstance()->SoundPlayWave(edamameBGM_, false);
			}
		}
		//枝豆知識
		Trivia();
	}

}

bool Edamame::OnCollisionRay()
{
	return playerCamera_->OnCollisionRay(localAABB_, worldTransform_.translate);
}

void Edamame::Trivia()
{
	//枝豆知識とBGMのvolume変更

	if (isPlaySound_) {

		edamameTrivia_->Update();

		Vector3 distance = Function::Distance(playerCamera_->GetRay().origin, worldTransform_.translate);
		float  length = Function::Length(distance);

		if (length >= 100.0f) {
			Audio::GetInstance()->SetSoundVolume(&edamameBGM_, 0.0f);
			edamameTrivia_->SetVol(0.0f);
		} else {

			if (length <= 1.0f) {
				Audio::GetInstance()->SetSoundVolume(&edamameBGM_, 0.25f);
				edamameTrivia_->SetVol(1.0f);
			} else {
				float vol = 1.0f / length;
				Audio::GetInstance()->SetSoundVolume(&edamameBGM_, vol * 0.25f);
				edamameTrivia_->SetVol(vol);
			}
		}
	}
}
