#include "StageManager.h"

#include <cassert>

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
	auto it = stageFactories_.find(stageName);
	assert(it != stageFactories_.end());
	assert(nextStage_ == nullptr);

	nextStage_ = it->second();
	nextStageName_ = stageName;
}

void StageManager::RegisterStage(const std::string& stageName, std::function<std::unique_ptr<BaseStage>()> factory) {
	assert(factory);
	stageFactories_[stageName] = std::move(factory);
}