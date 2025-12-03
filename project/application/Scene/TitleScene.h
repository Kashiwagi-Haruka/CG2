#pragma once
#include "Sprite.h"
#include "BaseScene.h"
class GameBase;
class TitleScene : public BaseScene{
	struct SpriteData {
		Sprite* sprite = nullptr;
		uint32_t handle = 0;
		Vector2 size = {100, 100};
		Vector2 rotate = {0, 0};
		Vector2 translate = {0, 0};
	};

	
	bool isGameEndRequest_;
	SpriteData logoSP_;


public: 
	TitleScene();
	~TitleScene() override;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
	bool GetIsSceneEnd() { return isSceneEnd_; }
};
