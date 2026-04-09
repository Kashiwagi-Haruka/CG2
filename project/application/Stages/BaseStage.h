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
};