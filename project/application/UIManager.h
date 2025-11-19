#pragma once
#include "Vector2.h"
#include <cstdint>
class Sprite;
class GameBase;
class UIManager {

	struct SpriteData {
		Sprite* sprite = nullptr;
		uint32_t handle = 0;
		Vector2 size = {100,100};
		Vector2 rotate = {100,100};
		Vector2 translate = {0,0};
	};


	SpriteData playerHpSPData;
	SpriteData titleSPData;
	SpriteData resultSPData;

	public:

	UIManager();
	~UIManager();
	void Initialize(GameBase* gameBase);
	void Update();
	void Draw(GameBase* gameBase);


};
