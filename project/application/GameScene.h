#pragma once
#include "DebugCamera.h"
#include "GameBase.h"
#include "Sprite.h"


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

	    Transform fenceTransform{
	        .scale{1, 1, 1},
            .rotate{0, 0, 0},
            .translate{0, 0, 0}
        };
	    Transform spriteTransform{
	        .scale{1, 1, 1},
            .rotate{0, 0, 0},
            .translate{0, 0, 0}
        };
	    Transform sprite2Transform{
	        .scale{1, 1, 1},
            .rotate{0, 0, 0},
            .translate{0, 0, 0}
        };
	    Matrix4x4 ParticleWorldMatrix{};
	    Matrix4x4 ParticleWVPMatrix{};
			Vector3 position[4]{
	        {100, 100, 0}, // 左上 (100,100)
	        {300, 100, 0}, // 右上
	        {300, 300, 0}, // 右下
	        {100, 300, 0}  // 左下
	    };
	    Vector3 position2[4]{
	        {200, 200, 0}, // 左上 (100,100)
	        {400, 200, 0}, // 右上
	        {400, 400, 0}, // 右下
	        {200, 400, 0}  // 左下
	    };
	    Vector2 texcoord[4] = {
	        {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}
        };

		DebugCamera camera;

		Sprite* sprite = nullptr;
	    Sprite* sprite2_ = nullptr;
		
		Vector2 spriteTexSize {};
	    Vector2 spriteTexSize2{};
		
	

	bool IsPKey = false;
	bool IsXButton = false;
	bool IsKeyboard = true;

	uint32_t color;

	char keys[256] = {0};
	char preKeys[256] = {0};

	
	
    // 修正: staticメンバーの初期化はconstでなければならないため、constを追加
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

	ModelData modelData;
	
};
