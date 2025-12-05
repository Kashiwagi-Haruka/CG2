#pragma once

#include "VertexData.h"
#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include "Matrix4x4.h"
#include "Vector4.h"
#include <string>
#include "Transform.h"

class Camera;
class Object3dCommon;
class Model;
class Object3d {

	struct alignas(256) TransformationMatrix {
		Matrix4x4 WVP;   // 64 バイト
		Matrix4x4 World; // 64 バイト
		                 // ここで自動的に 128 バイト分のパディングが入って、
		                 // sizeof(TransformationMatrix) == 256 になる
	};
	struct CameraforGPU{
		Vector3 worldPosition;

	};
	Transform transform_ = {
	    {1.0f, 1.0f, 1.0f},
	    {0.0f, 0.0f, 0.0f},
	    {0.0f, 0.0f, 0.0f},
	};
	Transform cameraTransform_ = {
	    {1.0f, 1.0f, 1.0f  }, // スケール
	    {0.3f, 0.0f, 0.0f  }, //
	    {0.0f, 4.0f, -10.0f}  //
	};
	struct DirectionalLight {
		Vector4 color;         // 16
		Vector3 direction;     // 12
		float intensity;       // 4
		Vector3 specularColor; // 12
		float pad;             // 4 → 合計 48バイト (HLSLと一致)
	};

	
	int color = 0xffffffff;

	Camera* camera_;

	Object3dCommon* obj3dCommon_;
	DirectionalLight* directionalLightData_ = nullptr;

	TransformationMatrix* transformationMatrixData_;

	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_ = nullptr;
	CameraforGPU* cameraData_ = nullptr;


	Model* model_ = nullptr;
	Matrix4x4 worldMatrix;
	Matrix4x4 worldViewProjectionMatrix;
	Matrix4x4 viewProjectionMatrix;

public:
	void Initialize(Object3dCommon* modelCommon);
	void Update();
	void Draw();

	void CreateResources();
	void SetModel(const std::string& filePath);

	void SetCamera(Camera* camera);

	void SetTranslate(Vector3 translate);
	void SetRotate(Vector3 Rotate);
	void SetScale(Vector3 Scale);
	void SetTransform(Transform transform) { transform_ = transform; }
	void SetLight(DirectionalLight* light) { directionalLightData_ = light; }

	Vector3 GetTranslate() { return transform_.translate; }
	Vector3 GetRotate() { return transform_.rotate; }
	Vector3 GetScale() { return transform_.scale; }
	Transform GetTransform() { return transform_; }
};