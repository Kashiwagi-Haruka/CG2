#pragma once
#include <imgui.h>
#include "Sprite.h"
#include "BaseScene.h"

class GameBase;
class ResultScene : public BaseScene{
	struct SpriteData {
		Sprite* sprite = nullptr;
		uint32_t handle = 0;
		Vector2 size = {100, 100};
		Vector2 rotate = {0, 0};
		Vector2 translate = {0, 0};
	};
	

	SpriteData logoSP_;
	bool isGameover_;
	bool isGameClear_;
	Sprite* pressSpaceSprite = nullptr;
	uint32_t pressSpaceHandle = 0;
	Vector2 pressSpacePos = {640, 420}; // 中央寄り & 360 より下
	Vector2 pressSpaceSize = {300, 300};

	public:
	
	ResultScene();
	~ResultScene() override;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
	void SetOverOrClear(bool isGameClear, bool isGameOver) {
		isGameClear_ = isGameClear;
		isGameover_ = isGameOver;
	};
	bool GetIsSceneEnd() { return isSceneEnd_; }
};
