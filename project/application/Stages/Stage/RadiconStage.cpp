#include "RadiconStage.h"
#include "GameObject/YoshidaMath/CollisionManager/CollisionManager.h"
#include "GameObject/GameCamera/PlayerCamera/PlayerCamera.h"

RadiconStage::RadiconStage(Player* player) : player_(player) {
	radicon_ = std::make_unique<Radicon>();
}

void RadiconStage::Initialize() { 
	radicon_->Initialize();
}

void RadiconStage::SetPlayer(Player* player) { player_ = player; }

void RadiconStage::SetCollisionManager([[maybe_unused]] CollisionManager* collisionManager) {


}

void RadiconStage::UpdateGameObject([[maybe_unused]] Camera* camera, [[maybe_unused]] const Vector3& lightDirection, [[maybe_unused]] Player* player) {}

void RadiconStage::UpdatePortal() {}

void RadiconStage::CheckCollision() {}

void RadiconStage::DrawModel([[maybe_unused]] bool isShadow, [[maybe_unused]] bool drawPortal, [[maybe_unused]] bool isDrawParticle) {}

void RadiconStage::SetSceneCameraForDraw([[maybe_unused]] Camera* camera) {}

void RadiconStage::SetPlayerCamera([[maybe_unused]] PlayerCamera* playerCamera) { radicon_->SetCamera(playerCamera->GetCamera()); }

PortalManager* RadiconStage::GetPortalManager() { return nullptr; }

void RadiconStage::SetLightManager(Yoshida::LightManager* lightManager) { lightManager_ = lightManager; }

bool RadiconStage::CheckHitPlayerByStageHazard([[maybe_unused]] const Vector3& playerPosition, [[maybe_unused]] float playerRadius, [[maybe_unused]] float minHitSpeed) const { return false; }