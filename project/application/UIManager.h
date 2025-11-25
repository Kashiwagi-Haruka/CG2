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
		Vector2 rotate = {0,0};
		Vector2 translate = {0,0};
	};

	SpriteData playerHpSPData;
	SpriteData playerHPFlameSPData;

	int playerHP;
	int playerHPMax = 100;
	Vector2 playerHPMaxSize = {20,50};

	public:

	UIManager();
	~UIManager();
	void Initialize();
	void Update();
	void Draw();

	void SetPlayerPosition(Vector2 playerPosition);
	void SetPlayerHP(int HP);

};
