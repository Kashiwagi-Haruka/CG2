#pragma once
#include <imgui.h>
#include "Sprite.h"
#include "BaseScene.h"
#include <memory>
class GameBase;

class ResultScene : public BaseScene {

	struct SpriteData {
		std::unique_ptr<Sprite> sprite = nullptr;
		uint32_t handle = 0;
		Vector2 size = {100, 100};
		float rotate = 0;
		Vector2 translate = {0, 0};
	};

	SpriteData logoSP_;
	
	std::unique_ptr<Sprite> pressSpaceSprite = nullptr;
	uint32_t pressSpaceHandle = 0;

	Vector2 pressSpacePos = {640, 420};
	Vector2 pressSpaceSize = {300, 300};

public:
	ResultScene();
	~ResultScene() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
};
