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
	}
}

bool Edamame::OnCollisionRay()
{
	return playerCamera_->OnCollisionRay(localAABB_, worldTransform_.translate);
}
