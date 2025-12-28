#pragma once

#include "VertexData.h"
#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include "Matrix4x4.h"
#include "Vector4.h"
#include <string>
#include "Transform.h"
#include "Light/DirectionalLight.h"
#include "Light/PointLight.h"
#include "CameraForGPU.h"
class Camera;
class Object3dCommon;
class Model;
class Object3d {

	struct alignas(256) TransformationMatrix {
		Matrix4x4 WVP;   // 64 バイト
		Matrix4x4 World; // 64 バイト
		Matrix4x4 WorldInverseTranspose; // ここで自動的に 128 バイト分のパディングが入って、
		                 // sizeof(TransformationMatrix) == 256 になる
	};

	Transform transform_ = {
	    {1.0f, 1.0f, 1.0f},
	    {0.0f, 0.0f, 0.0f},
	    {0.0f, 0.0f, 0.0f},
	};

	

	Camera* camera_;

	Object3dCommon* obj3dCommon_;
	

	TransformationMatrix* transformationMatrixData_;

	Microsoft::WRL::ComPtr<ID3D12Resource> transformResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
	CameraForGpu* cameraData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_;
	

	Model* model_ = nullptr;
	Matrix4x4 worldMatrix;
	Matrix4x4 worldViewProjectionMatrix;
	

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

	void SetColor(Vector4 color);
	void SetEnableLighting(bool enable);
	void SetUvTransform(const Matrix4x4& uvTransform);
	void SetShininess(float shininess);
	void SetPointLight(PointLight pointlight);
	Vector3 GetTranslate() { return transform_.translate; }
	Vector3 GetRotate() { return transform_.rotate; }
	Vector3 GetScale() { return transform_.scale; }
	Transform GetTransform() { return transform_; }
};