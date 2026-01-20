#pragma once
#include "CameraForGPU.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "Vector4.h"
#include "VertexData.h"
#include <cstdint>
#include <d3d12.h>
#include <vector>
#include <wrl.h>

class Camera;

class Primitive {

public:
	enum PrimitiveName {
		Plane,
		Circle,
		Ring,
		Sphere,
		Torus,
		Cylinder,
		Cone,
		Line,
		Triangle,
		Box,
	};

private:
	PrimitiveName primitiveName_;
	struct Material {
		Vector4 color;
		int enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
		float shininess;
		float environmentCoefficient;
		float padding2[2];
	};
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
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	Material* materialData_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	std::vector<VertexData> vertices_;
	std::vector<uint32_t> indices_;
	uint32_t textureIndex_ = 0;
	Matrix4x4 worldMatrix;
	Matrix4x4 worldViewProjectionMatrix;
	bool isUseSetWorld;

public:
	void Initialize(PrimitiveName name);
	void Initialize(PrimitiveName name,const std::string& texturePath);
	void Update();
	void Draw();

	void SetCamera(Camera* camera);
	void SetTranslate(Vector3 translate);
	void SetRotate(Vector3 rotate);
	void SetScale(Vector3 scale);
	void SetTransform(Transform transform);
	void SetWorldMatrix(Matrix4x4 matrix);
	void SetLinePositions(const Vector3& start, const Vector3& end);
	void SetColor(Vector4 color);
	void SetEnableLighting(bool enable);
	void SetUvTransform(const Matrix4x4& uvTransform);
	void SetShininess(float shininess);
	void SetEnvironmentCoefficient(float coefficient);

private:
	Vector3 lineStart_ = {0.0f, 0.0f, 0.0f};
	Vector3 lineEnd_ = {0.0f, 0.0f, 0.0f};
	bool useLinePositions_ = false;
};