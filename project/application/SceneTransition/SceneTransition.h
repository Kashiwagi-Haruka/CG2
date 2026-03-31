#pragma once
#include "Sprite.h"
#include "Text/Text.h"
#include "Vector2.h"
#include <cstdint>
#include <memory>
#include <vector>

class SceneTransition {

	struct SpriteData {
		std::unique_ptr<Sprite> sprite = nullptr;
		uint32_t handle = 0;
		Vector2 size = {100, 100};
		Vector2 rotate = {0, 0};
		Vector2 translate = {0, 0};
	};

	struct Block {
		Vector2 position{0.0f, 0.0f};
		float delay = 0.0f;
		float size = 0.0f;
	};

	enum class Phase { Spread, HoldAndType, Reverse };

	SpriteData blockSpriteData_;
	std::vector<Block> blocks_;
	Text errorText_;
	uint32_t fontHandle_ = 0;
	bool typeStarted_ = false;

	float phaseTimer_ = 0.0f;
	float holdTimer_ = 0.0f;
	float textStartDelay_ = 0.2f;
	float afterTextWait_ = 0.6f;
	float spreadDuration_ = 1.0f;
	float reverseDuration_ = 0.55f;
	float blockDelayMax_ = 0.45f;
	float minBlockSize_ = 56.0f;
	float maxBlockSize_ = 180.0f;

	bool isIn_ = true;
	bool isEnd = false;
	Phase phase_ = Phase::Spread;

	void StartSpread(bool reverseStart);
	void SetupBlocks();
	void UpdateSpread();
	void UpdateReverse();
	void UpdateHoldAndType();
	float GetPhaseRatio(float duration) const;

public:
	SceneTransition();
	~SceneTransition();

	void Initialize(bool isIn);
	void Update();
	void Draw();
	bool IsEnd() { return isEnd; };
};