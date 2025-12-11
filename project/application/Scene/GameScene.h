#pragma once
#include "GameBase.h"
#include "Sprite.h"
#include "Object3d.h"
#include "Particles.h"
#include <imgui.h>
#include "Object/MapchipField.h"
#include "Audio.h"
#include "Object/Goal.h"
#include "SceneTransition.h"
#include "UIManager.h"
#include "Object/Background/Background.h"
#include "Object/CatchBlock.h"
#include "BaseScene.h"
#include "Object/Player/BulletManager.h"
#include "Object/House/House.h"

class Player;
class Enemy;
class CameraController;
class SkyDome;
class EnemyManager;


class GameScene: public BaseScene{


	private:
		


	bool IsPKey = false;
	bool IsXButton = false;
	bool IsKeyboard = true;

	uint32_t color;
	std::unique_ptr<Particles> particles;
	std::unique_ptr<UIManager> uimanager;
	std::unique_ptr<SceneTransition> sceneTransition;
	std::unique_ptr<Goal> goal;
	std::unique_ptr<Player> player;
	std::unique_ptr<EnemyManager> enemyManager;
	std::unique_ptr<SkyDome> skyDome;
	std::unique_ptr<CameraController> cameraController;
	std::unique_ptr<MapchipField> field;
	std::unique_ptr<House> house;
	std::unique_ptr<Background> BG;
	std::unique_ptr<BulletManager> bulletManager_;

	SoundData BGMData;

	// レベルアップ選択専用スプライト
	std::unique_ptr<Sprite> levelupIcons[4]; // 0:Atk, 1:Speed, 2:HP, 3:Allow

    ImVec4 meshColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // 初期値: 白
	DirectionalLight light{
	    .color{1, 1, 1, 1},
        .direction{0, 0, 1},
        .intensity{1}
    };
	
	bool goalActive = false; // 敵全滅後に true になる

	bool sceneEndClear = false;
	bool sceneEndOver = false;
	// =====================
	// レベルアップ選択用
	// =====================
	bool isLevelSelecting = false;
	int selectChoices[2]; // 0:Atk, 1:Speed, 2:HP, 3:Allow
	int cursorIndex = 0;  // 0 or 1

	public:

	GameScene();
	~GameScene() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
	bool IsSceneEnd_GameClear() { return sceneEndClear; }
	bool IsSceneEnd_GameOver() { return sceneEndOver; }


	
};
