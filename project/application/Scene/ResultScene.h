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
