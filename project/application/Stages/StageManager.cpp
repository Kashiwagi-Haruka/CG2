#include "StageManager.h"
#include "Stage/MirrorStage.h"

StageManager::StageManager(Player* player) : player_(player) {}

void StageManager::CreateStage(const std::string& sceneName) {
	if (sceneName == "MirrorStage") {
		stage_ = std::make_unique<MirrorStage>(player_);
	} else {
		stage_.reset();
	}
}

void StageManager::InitializeStage() {
	if (stage_) {
		stage_->Initialize();
	}
}

void StageManager::UpdateLight() {}
void StageManager::UpdatePlayerDamage() {}
void StageManager::UpdatePostEffect() {}

void StageManager::UpdateGameObject(Camera* camera, const Vector3& lightDirection) {
	if (stage_) {
		stage_->UpdateGameObject(camera, lightDirection, player_);
	}
}

void StageManager::UpdatePortal() {
	if (stage_) {
		stage_->UpdatePortal();
	}
}

void StageManager::CheckCollision(CollisionManager* collisionManager) {
	if (stage_) {
		stage_->CheckCollision(collisionManager);
	}
}

void StageManager::DrawModel(bool isShadow, bool drawPortal, bool isDrawParticle) {
	if (stage_) {
		stage_->DrawModel(isShadow, drawPortal, isDrawParticle);
	}
}

void StageManager::DrawDamageOverlay() {}

void StageManager::SetSceneCameraForDraw(Camera* camera) {
	if (stage_) {
		stage_->SetSceneCameraForDraw(camera);
	}
}

void StageManager::SetPlayerCamera(PlayerCamera* playerCamera) {
	if (stage_) {
		stage_->SetPlayerCamera(playerCamera);
	}
}

PortalManager* StageManager::GetPortalManager() {
	if (!stage_) {
		return nullptr;
	}
	return stage_->GetPortalManager();
}
std::unique_ptr<CollisionManager> StageManager::GetCollisionManager() {
	if (!stage_) {
		return {};
	}
	return stage_->GetCollisionManager();
}
PointCommonLight* StageManager::GetPointLights() {
	if (!stage_) {
		return nullptr;
	}
	return stage_->GetPointLights();
}

uint32_t StageManager::GetActivePointLightCount() const {
	if (!stage_) {
		return 0;
	}
	return stage_->GetActivePointLightCount();
}

SpotCommonLight* StageManager::GetSpotLights() {
	if (!stage_) {
		return nullptr;
	}
	return stage_->GetSpotLights();
}

uint32_t StageManager::GetActiveSpotLightCount() const {
	if (!stage_) {
		return 0;
	}
	return stage_->GetActiveSpotLightCount();
}

AreaCommonLight* StageManager::GetAreaLights() {
	if (!stage_) {
		return nullptr;
	}
	return stage_->GetAreaLights();
}

uint32_t StageManager::GetActiveAreaLightCount() const {
	if (!stage_) {
		return 0;
	}
	return stage_->GetActiveAreaLightCount();
}