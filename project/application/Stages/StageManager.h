#pragma once

#include "BaseStage.h"
#include <memory>
#include <string>

class Player;

class StageManager {
private:
	std::unique_ptr<BaseStage> stage_ = nullptr;
	std::unique_ptr<BaseStage> nextStage_ = nullptr;
	Player* player_ = nullptr;
	std::string currentStageName_;
	std::string nextStageName_;

	std::unique_ptr<BaseStage> CreateStage(const std::string& stageName) const;

public:
	void Finalize();
	void Update();
	void Draw();

	void SetPlayer(Player* player) { player_ = player; }
	void ChangeStage(const std::string& stageName);
	bool IsCurrentEventRunning() const;

	const std::string& GetCurrentStageName() const { return currentStageName_; }
	float GetPlayerHp() const;
	float GetPlayerMaxHp() const;
	bool DidPlayerTakeDamage() const;
	bool IsPlayerDead() const;
};