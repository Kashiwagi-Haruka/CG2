#pragma once
#include "GameBase.h"
#include "Sprite.h"
#include "Object3d.h"
#include "ParticleEmitter.h"
#include <imgui.h>

class Player;
class Enemy;
class CameraController;
class Skydome;

class GameScene {

public:
	void Initialize(GameBase* gameBase);
	void Update(GameBase* gameBase);
	void Draw(GameBase* gameBase);
	~GameScene();

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
	CameraController* cameraController = nullptr;
	Skydome* skydome = nullptr;
    
    ImVec4 meshColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // 初期値: 白
	DirectionalLight light{
	    .color{1, 1, 1, 1},
        .direction{0, 0, 1},
        .intensity{1}
    };
	Matrix4x4 viewProjectionMatrix;

	// World行列を作る（必要に応じてGameBaseからTransformを使ってもよい）
	Matrix4x4 worldMatrix;
	Matrix4x4 fenceWorldMatrix;
	
	// WVP行列を作成
	Matrix4x4 wvpMatrix;
	Matrix4x4 fenceWvpMatrix;
	
};
