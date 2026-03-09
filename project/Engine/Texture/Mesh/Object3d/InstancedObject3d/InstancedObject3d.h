#pragma once
#include "Camera.h"
#include "Matrix4x4.h"
#include "PSO/CreatePSO.h"
#include "Vector3.h"
#include "Vector4.h"
#include <cstdint>
#include <d3d12.h>
#include <memory>
#include <string>
#include <vector>
#include <wrl.h>

class Model;

struct ID3D12Resource;

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

	struct InstanceData {
		Vector3 position;
		float yaw;
		float scale;
		Vector3 padding;
	};

	static constexpr uint32_t kCoffeeInstanceCount_ = 4096;

	std::vector<CoffeeVertex> coffeeVertices_{};
	std::vector<uint32_t> coffeeIndices_{};

	std::unique_ptr<CreatePSO> pso_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> instanceDataResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> sceneConstantResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> collisionParamResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> collisionRootSignature_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> collisionPipelineState_ = nullptr;
	CoffeeSceneConstants* sceneConstants_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	uint32_t instanceSrvIndex_ = 0;
	uint32_t instanceDataUavIndex_ = 0;
	uint32_t instanceWorldUavIndex_ = 0;
	bool hasInstanceSrvIndex_ = false;
	bool hasCollisionDescriptor_ = false;
	std::string modelPath_{};
	bool isInitialized_ = false;

	void CreateDefaultMesh();
	void CreateMeshFromModel(const Model& model);
	void CreateMesh();
	void CreateInstancingPipeline();
	void CreateCollisionPipeline();
	void CreateBuffers();
	void DispatchCollision();

public:
	void Initialize(const std::string& modelPath = "");
	void SetModel(const std::string& modelPath);
	void Update(const Camera* camera, const Vector3& lightDirection);
	void Draw();
	uint32_t GetInstanceCount() const { return kCoffeeInstanceCount_; }
};