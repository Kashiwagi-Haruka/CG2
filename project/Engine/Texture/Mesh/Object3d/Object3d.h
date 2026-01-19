#pragma once

#include "CameraForGPU.h"
#include "Light/DirectionalLight.h"
#include "Matrix4x4.h"
#include "Animation/Animation.h"
#include "Transform.h"
#include "Vector4.h"
#include "VertexData.h"
#include <Windows.h>
#include <d3d12.h>
#include <string>
#include <wrl.h>
class Camera;

class Model;
class Object3d {

	struct alignas(256) TransformationMatrix {
		Matrix4x4 WVP;                   // 64 バイト
		Matrix4x4 World;                 // 64 バイト
		Matrix4x4 WorldInverseTranspose; // ここで自動的に 128 バイト分のパディングが入って、
		                                 // sizeof(TransformationMatrix) == 256 になる
	};

	Transform transform_ = {
	    {1.0f, 1.0f, 1.0f},
	    {0.0f, 0.0f, 0.0f},
	    {0.0f, 0.0f, 0.0f},
	};

	Camera* camera_;

	TransformationMatrix* transformationMatrixData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformResource_;
	CameraForGpu* cameraData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_;
	Model* model_ = nullptr;
	Matrix4x4 worldMatrix;
	Matrix4x4 worldViewProjectionMatrix;
	bool isUseSetWorld;
	const Animation::AnimationData* animation_ = nullptr;
	float animationTime_ = 0.0f;
	bool isLoopAnimation_ = true;

public:
	void Initialize();
	void Update();
	void Draw();

	void CreateResources();
	void SetModel(const std::string& filePath);

	void SetCamera(Camera* camera);

	void SetTranslate(Vector3 translate);
	void SetRotate(Vector3 Rotate);
	void SetScale(Vector3 Scale);
	void SetTransform(Transform transform) {
		transform_ = transform;
		isUseSetWorld = false;
	}
	void SetWorldMatrix(Matrix4x4 matrix) {
		worldMatrix = matrix;
		isUseSetWorld = true;
	}
	void SetColor(Vector4 color);
	void SetEnableLighting(bool enable);
	void SetUvTransform(const Matrix4x4& uvTransform);
	void SetShininess(float shininess);
	void SetEnvironmentCoefficient(float coefficient);
	void SetAnimation(const Animation::AnimationData* animation, bool loop = true) {
		animation_ = animation;
		isLoopAnimation_ = loop;
		animationTime_ = 0.0f;
	}
	void ResetAnimationTime(float time = 0.0f) { animationTime_ = time; }
	Vector3 GetTranslate() { return transform_.translate; }
	Vector3 GetRotate() { return transform_.rotate; }
	Vector3 GetScale() { return transform_.scale; }
	Transform GetTransform() { return transform_; }
	const Matrix4x4& GetWorldMatrix() const { return worldMatrix; }
};