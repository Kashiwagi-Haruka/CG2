#pragma once

class StageManager;
class Player;

class BaseStage {
protected:
	StageManager* stageManager_ = nullptr;
	bool isStageEnd_ = false;

public:
	virtual ~BaseStage() = default;

	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void Finalize() = 0;

	virtual void SetStageManager(StageManager* stageManager) { stageManager_ = stageManager; }
	virtual void SetPlayer(Player* player) { (void)player; }
	virtual bool GetIsStageEnd() const { return isStageEnd_; }
	virtual bool IsCurrentEventRunning() const { return false; }
	virtual float GetPlayerHp() const { return 0.0f; }
	virtual float GetPlayerMaxHp() const { return 0.0f; }
	virtual bool DidPlayerTakeDamage() const { return false; }
	virtual bool IsPlayerDead() const { return false; }
};
};