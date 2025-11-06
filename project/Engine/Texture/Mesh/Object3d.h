#pragma once

#include "VertexData.h"
#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include "Matrix4x4.h"
#include "Vector4.h"
#include <string>

class Object3dCommon;
class Model;
class Object3d {

	
	
	struct Transform {

		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};

	struct alignas(256) TransformationMatrix {
		Matrix4x4 WVP;   // 64 バイト
		Matrix4x4 World; // 64 バイト
		                 // ここで自動的に 128 バイト分のパディングが入って、
		                 // sizeof(TransformationMatrix) == 256 になる
	};

	Transform transform_ = {
	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f},
	};
	Transform cameraTransform_ = {
	    {1.0f, 1.0f, 1.0f  }, // スケール
	    {0.3f, 0.0f, 0.0f  }, //
	    {0.0f, 4.0f, -10.0f}  //
	};
	struct DirectionalLight {
		Vector4 color;
		Vector3 direction;
		float intensity;
	};


	int color = 0xffffffff;




	Object3dCommon* obj3dCommon_;
	DirectionalLight* directionalLightData_ = nullptr;
	
	TransformationMatrix* transformationMatrixData_;

	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;


	Model* model_ = nullptr;

	public:

	void Initialize(Object3dCommon* modelCommon);
	    void Update();
	void Draw();
	
	void CreateResources();
	void SetModel(const std::string& filePath);

	void SetTranslate(Vector3 translate);
	void SetRotate(Vector3 Rotate);
	void SetScale(Vector3 Scale);

	Vector3 GetTranslate() { return transform_.translate; }
	Vector3 GetRotate() { return transform_.rotate; }
	Vector3 GetScale() { return transform_.scale; }
};
