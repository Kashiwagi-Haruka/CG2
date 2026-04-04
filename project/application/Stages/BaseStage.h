#pragma once

class StageManager;

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
	virtual bool GetIsStageEnd() const { return isStageEnd_; }
};