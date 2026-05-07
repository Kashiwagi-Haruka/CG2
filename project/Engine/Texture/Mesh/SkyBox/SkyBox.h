#pragma once
#include "CameraForGPU.h"
#include "Data/Material.h"
#include "Data/VertexData.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "Vector3.h"
#include <d3d12.h>
#include <string>
#include <vector>
#include <wrl.h>
class Camera;
class SkyBox {

private:
	struct alignas(256) TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 DirectionalLightWVP;
		Matrix4x4 PointLightWVP;
		Matrix4x4 SpotLightWVP;
		Matrix4x4 AreaLightWVP;
		Matrix4x4 World;
		Matrix4x4 WorldInverseTranspose;
	};
	Transform transform_{};
	Camera* camera_ = nullptr;
	uint32_t textureIndex_ = 0;
	std::vector<VertexData> vertices_{};
	std::vector<uint32_t> indices_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> transformResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	TransformationMatrix* transformationMatrixData_ = nullptr;
	CameraForGpu* cameraData_ = nullptr;
	Material* materialData_ = nullptr;

public:
	SkyBox();
	~SkyBox();
	void Initialize();
	void SetCamera(Camera* camera);
	void SetDDSTexture(const std::string& filePath);
	void SetScale(const Vector3& scale);
	void SetRotation(const Vector3& rotation);
	void Update();
	void Draw();
};