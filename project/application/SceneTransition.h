#pragma once
#include "Sprite.h"
#include "Vector2.h"
#include <cstdint>
#include <memory>
class SceneTransition {

	struct SpriteData {
		std::unique_ptr<Sprite> sprite = nullptr;
		uint32_t handle = 0;
		Vector2 size = {100, 100};
		Vector2 rotate = {0, 0};
		Vector2 translate = {0, 0};
	};

	SpriteData fadeSPData;
	float color = 0.0f;
	bool isEnd;

public:
	SceneTransition();
	~SceneTransition();

	void Initialize();
	void Update(bool isIn);
	void Draw();


};
