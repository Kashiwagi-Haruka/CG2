#pragma once

#include "BaseStage.h"
#include <memory>
#include <string>

class StageManager {
private:
	std::unique_ptr<BaseStage> stage_ = nullptr;
	std::unique_ptr<BaseStage> nextStage_ = nullptr;
	std::string currentStageName_;
	std::string nextStageName_;

	std::unique_ptr<BaseStage> CreateStage(const std::string& stageName) const;

public:
	void Finalize();
	void Update();
	void Draw();

	void ChangeStage(const std::string& stageName);

	const std::string& GetCurrentStageName() const { return currentStageName_; }
};