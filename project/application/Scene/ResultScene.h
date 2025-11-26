#pragma once
#include <imgui.h>
#include "Sprite.h"

class GameBase;
class ResultScene {
	struct SpriteData {
		Sprite* sprite = nullptr;
		uint32_t handle = 0;
		Vector2 size = {100, 100};
		Vector2 rotate = {0, 0};
		Vector2 translate = {0, 0};
	};
	bool isSceneEnd_;

	SpriteData logoSP_;

	public:
	
	ResultScene();
	~ResultScene();
	void Initialize();
	void Update(bool isGameClear, bool isGameOver);
	void Draw();
	bool GetIsSceneEnd() { return isSceneEnd_; }
};
