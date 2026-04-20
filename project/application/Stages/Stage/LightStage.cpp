#include "LightStage.h"
#include "GameObject/YoshidaMath/CollisionManager/CollisionManager.h"

LightStage::LightStage(Player* player) : player_(player) {}

void LightStage::Initialize() {}

void LightStage::SetPlayer(Player* player) { player_ = player; }

void LightStage::SetCollisionManager([[maybe_unused]] CollisionManager* collisionManager) {}

void LightStage::UpdateGameObject([[maybe_unused]] Camera* camera, [[maybe_unused]] const Vector3& lightDirection, [[maybe_unused]] Player* player) {}

void LightStage::UpdatePortal() {}

void LightStage::CheckCollision() {}

void LightStage::DrawModel([[maybe_unused]] bool isShadow, [[maybe_unused]] bool drawPortal, [[maybe_unused]] bool isDrawParticle) {}

void LightStage::SetSceneCameraForDraw([[maybe_unused]] Camera* camera) {}

void LightStage::SetPlayerCamera([[maybe_unused]] PlayerCamera* playerCamera) {}

PortalManager* LightStage::GetPortalManager() { return nullptr; }

