#pragma once
#include "GameBase.h"
#include "VertexData.h"  
#include "Function.h"

class ObjDraw {

	Function fn;

	struct Transform {

		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};
	
	struct DirectionalLight {
		Vector4 color;
		Vector3 direction;
		float intensity;
	};

	struct Sphere{

		Transform transform;
		DirectionalLight directionalLight;

	};
	struct Sprite {
	
		Transform transform;   // スプライトの変形情報
		Vector3 texturePos[4]; // テクスチャ座標
		Vector3 worldTexturePos[4]; // ワールド座標系でのテクスチャ座標
		Vector2 UV[4];
		Transform UVTransform; // UV変形情報（スケール・回転・移動）
		
	};
	struct Object {
		Transform transform;   // オブジェクトの変形情報
		Vector2 texturePos[4]; // テクスチャ座標
	};

	ModelData planeModel;
	Sprite sprite;
	Sphere sphere;
	Object object;
	
	public:


	void Initialize(GameBase& gameBase);
	void DrawObjSprite(GameBase& gameBase,const Matrix4x4 viewProj);
	void DrawSphere(GameBase& gameBase, const Matrix4x4 viewProj);
	void HarfLightControl(GameBase& gameBase);

};
