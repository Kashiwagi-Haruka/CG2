#pragma once
#include "Vector2.h"
#include <cstdint>
#include "Object/Player/PlayerParameters.h"
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
		OperateCountMAX,

	};//操作方法
	enum Numbers {

		kExp100,
		kExp10,
		kEexp1,
		kExpMax100,
		kExpMax10,
		kExpMax1,
		kLv,
		kAttuck,
		kHealth,
		kSpeed,
		kArrow,
		NumbersCountMax
	};
	struct SpriteData {
		Sprite* sprite = nullptr;
		uint32_t handle = 0;
		Vector2 size = {100,100};
		Vector2 rotate = {0,0};
		Vector2 translate = {0,0};
	};

	SpriteData playerHpSPData;
	SpriteData playerHPFlameSPData;
	SpriteData HowtoOperateSPData[OperateCountMAX];
	SpriteData LevelSPData;
	SpriteData NumberSPData[NumbersCountMax];
	SpriteData MaxSPData;
	SpriteData AttuckUpSPData;
	SpriteData HealthUpSPData;
	SpriteData SpeedUpSPData;
	SpriteData AllowUpSPData;

	int playerHP;
	int playerHPMax;
	Vector2 playerHPMaxSize = {20,50};
	Vector2 numbersTextureSize = {300, 300};
	Parameters parameters_;

	public:

	UIManager();
	~UIManager();
	void Initialize();
	void Update();
	void Draw();

	void SetPlayerPosition(Vector2 playerPosition);
	void SetPlayerHP(int HP);
	void SetPlayerHPMax(int HPMax);
	void SetPlayerParameters(Parameters parameters);
};
