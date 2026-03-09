#pragma once
#include "Camera.h"
#include "Matrix4x4.h"
#include "Vector3.h"
#include "Vector4.h"
#include <cstdint>
#include <d3d12.h>
#include <vector>
#include <wrl.h>

struct ID3D12PipelineState;
struct ID3D12Resource;
struct ID3D12RootSignature;

class InstancedObject3d {
private:
	struct CoffeeVertex {
		Vector3 position;
		Vector3 normal;
	};

	struct CoffeeSceneConstants {
		Matrix4x4 viewProjection;
		Vector4 lightDirection;
	};

	static constexpr uint32_t kCoffeeInstanceCount_ = 100000;

	std::vector<CoffeeVertex> coffeeVertices_{};
	std::vector<uint32_t> coffeeIndices_{};

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> sceneConstantResource_ = nullptr;
	CoffeeSceneConstants* sceneConstants_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	uint32_t instanceSrvIndex_ = 0;

	void CreateMesh();
	void CreateInstancingPipeline();
	void CreateBuffers();

public:
	void Initialize();
	void Update(const Camera* camera, const Vector3& lightDirection);
	void Draw();
	uint32_t GetInstanceCount() const { return kCoffeeInstanceCount_; }
};