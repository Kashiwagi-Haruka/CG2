#include "LightStage.h"
#include "GameObject/YoshidaMath/CollisionManager/CollisionManager.h"
#include "GameObject/Flashlight/Flashlight.h"

LightStage::LightStage(Player* player) : player_(player) {

	//懐中電灯の作成
	flashlight_ = std::make_unique<Flashlight>();
	flashlight_->SetPlayer(player_);

}

void LightStage::Initialize() {

	//懐中電灯の初期化
	flashlight_->Initialize();

	//ライトの設定
	lightManager_->SetActiveLightCount(Yoshida::LightManager::SPOT, 1);
	lightManager_->SetSpotLight(flashlight_->GetSpotLight(), 0);

}

void LightStage::SetPlayer(Player* player) { 
	player_ = player;
	flashlight_->SetPlayer(player_);
}

void LightStage::SetCollisionManager([[maybe_unused]] CollisionManager* collisionManager) { stageCollisionManager_ = collisionManager; }

void LightStage::UpdateGameObject([[maybe_unused]] Camera* camera, [[maybe_unused]] const Vector3& lightDirection, [[maybe_unused]] Player* player) {


	//懐中電灯の更新
	flashlight_->Update();

	//ライトの更新
	lightManager_->SetSpotLight(flashlight_->GetSpotLight(), 0);

}

void LightStage::UpdatePortal() {}

void LightStage::CheckCollision() {

	stageCollisionManager_->AddCollider(flashlight_.get());

}

void LightStage::DrawModel([[maybe_unused]] bool isShadow, [[maybe_unused]] bool drawPortal, [[maybe_unused]] bool isDrawParticle) {

	flashlight_->Draw();
}
void LightStage::DrawSprite()
{

}
void LightStage::SetSceneCameraForDraw([[maybe_unused]] Camera* camera) {
	flashlight_->SetCamera(camera);
}

void LightStage::SetPlayerCamera([[maybe_unused]] PlayerCamera* playerCamera) {
	flashlight_->SetPlayerCamera(playerCamera);
}

PortalManager* LightStage::GetPortalManager() { return nullptr; }

bool LightStage::CheckHitPlayerByStageHazard([[maybe_unused]] const Vector3& playerPosition, [[maybe_unused]] float playerRadius, [[maybe_unused]] float minHitSpeed) const { return false; }

