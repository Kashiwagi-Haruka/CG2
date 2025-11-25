#pragma once
#include "GameBase.h"
#include "Sprite.h"
#include "Object3d.h"
#include "ParticleEmitter.h"
#include <imgui.h>
#include "MapchipField.h"
#include "Audio.h"
#include "Goal.h"
#include "SceneTransition.h"
#include "UIManager.h"

class Player;
class Enemy;
class CameraController;
class SkyDome;

class GameScene {


	private:
		
	ParticleEmitter* particle = nullptr;


	bool IsPKey = false;
	bool IsXButton = false;
	bool IsKeyboard = true;

	uint32_t color;

	UIManager* uimanager = nullptr;
	SceneTransition* sceneTransition = nullptr;
	Goal* goal = nullptr;
	Player* player = nullptr;
	Enemy* enemy = nullptr;
	SkyDome* skyDome = nullptr;
	CameraController* cameraController = nullptr;
	MapchipField* field = nullptr;
	SoundData soundData;
    
    ImVec4 meshColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // 初期値: 白
	DirectionalLight light{
	    .color{1, 1, 1, 1},
        .direction{0, 0, 1},
        .intensity{1}
    };
	

	bool sceneEndClear = false;
	bool sceneEndOver = false;

	public:

	GameScene();
	~GameScene();

	void Initialize();
	void Update();
	void Draw();
	bool IsSceneEnd_GameClear() { return sceneEndClear; }
	bool IsSceneEnd_GameOver() { return sceneEndOver; }


	
};
