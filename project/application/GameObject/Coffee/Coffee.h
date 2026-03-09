#pragma once
#include "Object3d/InstancedObject3d/InstancedObject3d.h"
#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl.h>

class Camera;

class Coffee {
public:
	Coffee();
	void Initialize();
	void Update(Camera* camera, const Vector3& lightDirection);
	void Draw();
	uint32_t GetInstanceCount() const;

private:
	struct InstanceData {
		Vector3 position;
		float yaw = 0.0f;
		float scale = 1.0f;
		float velocityY = 0.0f;
		float radius = 0.25f;
		float padding[2] = {0.0f, 0.0f};
	};

	struct alignas(256) SimulationParams {
		uint32_t instanceCount = 0;
		float deltaTime = 1.0f / 60.0f;
		float roomMinX = -10.0f;
		float roomMaxX = 10.0f;
		float roomMinZ = -10.0f;
		float roomMaxZ = 10.0f;
		float floorY = 0.0f;
		float gravity = -1.0f;
		float bounceDamping = 0.8f;
		float separationBias = 0.001f;
		float padding[2] = {0.0f, 0.0f};
	};

	void InitializeSimulationResources();
	void RunSimulation();

	std::unique_ptr<InstancedObject3d> instancedObject_ = nullptr;
	std::vector<InstanceData> instances_{};

	Microsoft::WRL::ComPtr<ID3D12RootSignature> simulationRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> simulationPipelineState_;
	Microsoft::WRL::ComPtr<ID3D12Resource> instanceDataResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> instanceDataUploadResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> instanceDataReadbackResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> simulationParamsResource_;
	SimulationParams* simulationParamsData_ = nullptr;
	uint32_t instanceDataUavIndex_ = 0;
};