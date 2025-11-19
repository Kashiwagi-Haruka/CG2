#pragma once
#include "GameBase.h"
#include "Sprite.h"
#include "Object3d.h"
#include "ParticleEmitter.h"
#include <imgui.h>
#include "MapchipField.h"
#include "Audio.h"

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

	void Initialize(GameBase* gameBase);
	void Update(GameBase* gameBase);
	void Draw(GameBase* gameBase);
	bool IsSceneEnd_GameClear() { return sceneEndClear; }
	bool IsSceneEnd_GameOver() { return sceneEndOver; }


	
};
