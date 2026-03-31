#pragma once
#include "Sprite.h"
#include <cstdint>
#include <memory>

class SceneTransition {

	std::unique_ptr<Sprite> fadeSprite_ = nullptr;
	uint32_t fadeTextureHandle_ = 0;

	bool isIn_ = true;
	bool isEnd = false;
	float timer_ = 0.0f;
	float duration_ = 0.35f;

	float GetProgress() const;

public:
	SceneTransition();
	~SceneTransition();

	void Initialize(bool isIn);
	void Update();
	void Draw();
	bool IsEnd() { return isEnd; };
};
