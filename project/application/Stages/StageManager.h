#pragma once

#include "BaseStage.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

class StageManager {
private:
	std::unique_ptr<BaseStage> stage_ = nullptr;
	std::unique_ptr<BaseStage> nextStage_ = nullptr;
	std::string currentStageName_;
	std::string nextStageName_;

	std::unordered_map<std::string, std::function<std::unique_ptr<BaseStage>()>> stageFactories_;

public:
	void Finalize();
	void Update();
	void Draw();

	void ChangeStage(const std::string& stageName);
	void RegisterStage(const std::string& stageName, std::function<std::unique_ptr<BaseStage>()> factory);

	const std::string& GetCurrentStageName() const { return currentStageName_; }
};