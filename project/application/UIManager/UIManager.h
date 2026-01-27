#pragma once
#include "Object/Player/PlayerParameters.h"
#include "Vector2.h"
#include <cstdint>
#include <memory>
class Sprite;
class GameBase;
class UIManager {

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
		std::unique_ptr<Sprite> sprite;
		uint32_t handle = 0;
		Vector2 size = {100, 100};
		Vector2 rotate = {0, 0};
		Vector2 translate = {0, 0};
	};

	SpriteData playerHpSPData;
	SpriteData playerHPFlameSPData;
	SpriteData HowtoOperateSPData;
	SpriteData LevelSPData;
	SpriteData NumberSPData[NumbersCountMax];
	SpriteData MaxSPData[5];
	SpriteData AttackUpSPData;
	SpriteData HealthUpSPData;
	SpriteData SpeedUpSPData;
	SpriteData AllowUpSPData;

	SpriteData SlashSPData[2];
	SpriteData EXPSPData;
	SpriteData houseHpNumberSPData[3];
	SpriteData houseHpPercentSPData;

	int playerHP;
	int playerHPMax;
	Vector2 playerHPMaxSize = {400, 100};
	float playerHPWidth = 1200.0f;
	float playerHPWidthMax = 1200.0f;
	Vector2 numbersTextureSize = {300, 300};
	Vector2 houseHpNumbersTextureSize = {400, 400};
	Vector2 houseHpPercentBasePosition = {980, 520};
	int houseHpDigitStartIndex = 0;
	Parameters parameters_;

public:
	UIManager();
	~UIManager();
	void Initialize();
	void Update();
	void Draw();

	void SetPlayerHP(int HP);
	void SetPlayerHPMax(int HPMax);
	void SetPlayerParameters(Parameters parameters);
};
