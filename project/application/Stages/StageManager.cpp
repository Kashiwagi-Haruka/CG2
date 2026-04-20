#include "Stage/LightStage.h"
#include "Stage/MirrorStage.h"
#include "StageManager.h"
#include"Stage/TutorialStage/TutorialStage.h"

StageManager::StageManager(Player* player) : player_(player) {}

void StageManager::CreateStage(const std::string& sceneName) {
    if (stage_) {
        stage_.reset();
    }

    if (sceneName == "MirrorStage") {
        stage_ = std::make_unique<MirrorStage>(player_);
    } else if (sceneName == "LightStage") {
        stage_ = std::make_unique<LightStage>(player_);
    } else if (sceneName == "TutorialStage") {
        stage_ = std::make_unique<TutorialStage>(player_);
    }
}
void StageManager::SetPlayer(Player* player) {
    player_ = player;
    if (stage_) {
        stage_->SetPlayer(player_);
    }
}
void StageManager::SetCollisionManager(CollisionManager* collisionManager) {
    if (stage_) {
        stage_->SetCollisionManager(collisionManager);
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

void StageManager::CheckCollision() {
    if (stage_) {
        stage_->CheckCollision();
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

void StageManager::SetLightManager(Yoshida::LightManager* lightManager)
{
    if (stage_) {
        stage_->SetLightManager(lightManager);
    }

}
