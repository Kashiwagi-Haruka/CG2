#pragma once
#include "Vector2.h"
#include <cstdint>
class Sprite;
class GameBase;
class UIManager {

	enum HowtoOperate{
		kW,
		kA,
		kS,
		kD,
		kSpace,
		kAttuckButton,
		CountMAX,

	};//操作方法

	struct SpriteData {
		Sprite* sprite = nullptr;
		uint32_t handle = 0;
		Vector2 size = {100,100};
		Vector2 rotate = {0,0};
		Vector2 translate = {0,0};
	};

	SpriteData playerHpSPData;
	SpriteData playerHPFlameSPData;
	SpriteData HowtoOperateSPData[CountMAX];

	int playerHP;
	int playerHPMax;
	Vector2 playerHPMaxSize = {20,50};

	public:

	UIManager();
	~UIManager();
	void Initialize();
	void Update();
	void Draw();

	void SetPlayerPosition(Vector2 playerPosition);
	void SetPlayerHP(int HP);
	void SetPlayerHPMax(int HPMax);
};
