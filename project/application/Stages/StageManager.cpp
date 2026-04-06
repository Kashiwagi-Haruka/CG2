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
		stage_->Initialize();
	}

	if (nextStage_ && !stage_) {
		stage_ = std::move(nextStage_);
		currentStageName_ = nextStageName_;
		nextStageName_.clear();
		stage_->SetStageManager(this);
		stage_->Initialize();
	}

	if (stage_) {
		stage_->Update();
	}
}

void StageManager::Draw() {
	if (stage_) {
		stage_->Draw();
	}
}

void StageManager::ChangeStage(const std::string& stageName) {
	auto nextStage = CreateStage(stageName);
	assert(nextStage != nullptr);
	assert(nextStage_ == nullptr);

	nextStage_ = std::move(nextStage);
	nextStageName_ = stageName;
}