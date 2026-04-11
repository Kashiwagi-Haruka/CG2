#include "StageManager.h"

#include "Stage/MirrorStage.h"

#include <cassert>

std::unique_ptr<BaseStage> StageManager::CreateStage(const std::string& stageName) const {
	if (stageName == "MirrorStage") {
		return std::make_unique<MirrorStage>();
	}

	return nullptr;
}

void StageManager::Finalize() {
	if (stage_) {
		stage_->Finalize();
		stage_.reset();
	}

	nextStage_.reset();
	currentStageName_.clear();
	nextStageName_.clear();
}

void StageManager::Update() {
	if (nextStage_ && stage_) {
		stage_->Finalize();
		stage_ = std::move(nextStage_);
		currentStageName_ = nextStageName_;
		nextStageName_.clear();
		stage_->SetStageManager(this);
		stage_->SetPlayer(player_);
		stage_->SetPlayerCamera(playerCamera_);
		stage_->SetElevatorManager(elevatorRoomManager_);
		stage_->SetDirectionalShadowEnabled(useDirectionalShadow_);
		stage_->Initialize();
	}

	if (nextStage_ && !stage_) {
		stage_ = std::move(nextStage_);
		currentStageName_ = nextStageName_;
		nextStageName_.clear();
		stage_->SetStageManager(this);
		stage_->SetPlayer(player_);
		stage_->SetPlayerCamera(playerCamera_);
		stage_->SetElevatorManager(elevatorRoomManager_);
		stage_->SetDirectionalShadowEnabled(useDirectionalShadow_);
		stage_->Initialize();
	}

	if (stage_) {
		stage_->Update();
	}
}

void StageManager::ShadowMapDraw() {
	if (stage_) {
		stage_->ShadowMapDraw();
	}
}

void StageManager::MainDraw() {
	if (stage_) {
		stage_->MainDraw();
	}
}
void StageManager::CheckCollision() {
	if (stage_) {
		stage_->CheckCollision();
	}
}
void StageManager::ChangeStage(const std::string& stageName) {
	auto nextStage = CreateStage(stageName);
	assert(nextStage != nullptr);
	assert(nextStage_ == nullptr);

	nextStage_ = std::move(nextStage);
	nextStageName_ = stageName;
}

void StageManager::SetDirectionalShadowEnabled(bool enabled) {
	useDirectionalShadow_ = enabled;
	if (stage_) {
		stage_->SetDirectionalShadowEnabled(enabled);
	}
	if (nextStage_) {
		nextStage_->SetDirectionalShadowEnabled(enabled);
	}
}
bool StageManager::IsCurrentEventRunning() const {
	if (!stage_) {
		return false;
	}

	return stage_->IsCurrentEventRunning();
}
float StageManager::GetPlayerHp() const {
	if (!stage_) {
		return 0.0f;
	}
	return stage_->GetPlayerHp();
}

float StageManager::GetPlayerMaxHp() const {
	if (!stage_) {
		return 0.0f;
	}
	return stage_->GetPlayerMaxHp();
}

bool StageManager::DidPlayerTakeDamage() const {
	if (!stage_) {
		return false;
	}
	return stage_->DidPlayerTakeDamage();
}

bool StageManager::IsPlayerDead() const {
	if (!stage_) {
		return false;
	}
	return stage_->IsPlayerDead();
}