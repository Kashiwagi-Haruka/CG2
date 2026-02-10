#pragma once
#include "BlendMode/BlendModeManager.h"
#include "Matrix4x4.h"
#include "RigidBody.h"
#include "Transform.h"
#include "Vector3.h"
#include "Vector4.h"
#include <array>
#include <cstdint>
#include <d3d12.h>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <wrl.h>

struct Particle {
	Transform transform_{
	    .scale = {1, 1, 1},
          .rotate{0, 0, 0},
          .translate{0, 0, 0}
    };

	Vector3 vel{};
	float life{};
	Vector4 color = {1, 1, 1, 1};
	float fadeSpeed = 0.02f;

	BlendMode blendmode = BlendMode::kBlendModeAlpha;

	Vector3 accel;
	AABB area;
	bool visible = true;
};

class SrvManager;
class DirectXCommon;
class Camera;

class ParticleManager {

public:
	struct ParticleGroup {
		std::string textureFilePath;
		uint32_t textureSrvIndex = 0;
		std::list<Particle> particles;
		uint32_t drawCount = 1024;
	};

public:
	static ParticleManager* GetInstance();
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);
	void CreateParticleGroup(const std::string& name, const std::string& textureFilePath);
	void Emit(const std::string& name, const Transform& transform, uint32_t count, const Vector3& accel, const AABB& area, float life);
	void SetCamera(Camera* camera);
	void SetBlendMode(BlendMode mode);

	void Update(Camera* camera);
	void Draw(const std::string& name);
	void Clear();
	void Finalize();

private:
	struct PerView {
		Matrix4x4 viewProjection;
		Matrix4x4 billboardMatrix;
	};

	static constexpr uint32_t kMaxParticles_ = 1024;

	static std::unique_ptr<ParticleManager> instance;
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	std::unordered_map<std::string, ParticleGroup> particleGroups;
	Camera* camera_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_[6];
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_;
	BlendModeManager blendModeManager_;
	BlendMode currentBlendMode_ = BlendMode::kBlendModeAlpha;

	Microsoft::WRL::ComPtr<ID3D12Resource> cbResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> perViewCB_;

	Microsoft::WRL::ComPtr<ID3D12Resource> particleResource_;
	uint32_t particleSrvIndex_ = 0;
	uint32_t particleUavIndex_ = 0;
	D3D12_RESOURCE_STATES particleResourceState_ = D3D12_RESOURCE_STATE_COMMON;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> computePipelineState_;
	bool isParticleInitialized_ = false;

	void CreateRootsignature();
	void CreateGraphicsPipeline();
	void CreateComputePipeline();
	void InitializeParticlesByCompute();
};