#pragma once
#include "Sprite.h"
class GameBase;
class TitleScene {
	struct SpriteData {
		Sprite* sprite = nullptr;
		uint32_t handle = 0;
		Vector2 size = {100, 100};
		Vector2 rotate = {0, 0};
		Vector2 translate = {0, 0};
	};

	bool isSceneEnd_;
	bool isGameEndRequest_;
	SpriteData logoSP_;


public: 
	TitleScene();
	~TitleScene();
	void Initialize();
	void Update();
	void Draw();
	bool GetIsSceneEnd() { return isSceneEnd_; }
};
