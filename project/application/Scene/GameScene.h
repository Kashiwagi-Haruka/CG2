#pragma once
#include "GameBase.h"
#include "Sprite.h"
#include "Object3d.h"
#include "ParticleEmitter.h"
#include <imgui.h>
#include "MapchipField.h"


class Player;
class Enemy;
class CameraController;
class SkyDome;

class GameScene {


	private:
	
	Transform planeTransform{
	        .scale{1, 1, 1},
            .rotate{0, 0, 0},
            .translate{0, 0, 0}
    };
	Transform axisTransform{
	        .scale{1, 1, 1},
            .rotate{0, 0, 0},
            .translate{0, 0, 0}
    };
	Transform fenceTransform{
	        .scale{1, 1, 1},
            .rotate{0, 0, 0},
            .translate{0, 0, 0}
    };
	Transform spriteTransform{
	        .scale{500, 500, 1},
            .rotate{0, 0, 0},
            .translate{100, 100, 0}
    };
	Transform sprite2Transform{
	        .scale{1, 1, 1},
            .rotate{0, 0, 0},
            .translate{0, 0, 0}
    };
		
	Matrix4x4 ParticleWorldMatrix{};
	Matrix4x4 ParticleWVPMatrix{};

	Sprite* sprite = nullptr;
	Sprite* sprite2_ = nullptr;
	uint32_t spriteHandle = 0;
	uint32_t spriteHandle2 = 0;
		
	Object3d* planeObject_ = nullptr;
	Object3d* axisObject_ = nullptr;
	    
		
	ParticleEmitter* particle = nullptr;

	Vector2 spriteTexSize {};
	Vector2 spriteTexSize2{};
		
	

	bool IsPKey = false;
	bool IsXButton = false;
	bool IsKeyboard = true;

	uint32_t color;



	uint32_t ModelTextureHandle = 0;

	Player* player = nullptr;
	Enemy* enemy = nullptr;
	SkyDome* skyDome = nullptr;
	CameraController* cameraController = nullptr;
	MapchipField* field = nullptr;
    
    ImVec4 meshColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // 初期値: 白
	DirectionalLight light{
	    .color{1, 1, 1, 1},
        .direction{0, 0, 1},
        .intensity{1}
    };
	

	int sceneEnd = false;

	public:

	GameScene();
	~GameScene();

	void Initialize(GameBase* gameBase);
	void Update(GameBase* gameBase);
	void Draw(GameBase* gameBase);
	bool IsSceneEnd() { return sceneEnd; }
	


	
};
