#pragma once
#include <string>
#include <unordered_map>
#include <list>
#include <wrl.h>
#include <d3d12.h>
#include "Vector3.h"
#include "Matrix4x4.h"
#include <cstdint>
#include "BlendModeManeger.h"
#include "Transform.h"
#include "RigidBody.h"
#include "Vector4.h"
#include <array>
#include <memory>

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

	// ★ 個別フィールド
	Vector3 accel; // 個別加速度
	AABB area;     // 個別発生エリア
	bool visible = true;
};


class SrvManager;
class DirectXCommon;
class Camera;

class ParticleManager {

public:

	struct ParticleGroup {

		std::string textureFilePath;
		uint32_t textureSrvIndex;

		std::list<Particle> particles;

		// ★ ブレンドモードごとのバッファ
		struct BlendBucket {
			uint32_t instancingSrvIndex = 0;
			Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
			void* instancingDataPtr = nullptr;
			uint32_t instanceCount = 0;
			uint32_t maxInstance = 1000;
		};

		std::array<BlendBucket, static_cast<int>(BlendMode::kCountOfBlendMode)> buckets;
	};


public:
	static ParticleManager* GetInstance();
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);
	void CreateParticleGroup(const std::string& name, const std::string& textureFilePath);
	void Emit(const std::string& name, const Transform& transform, uint32_t count, const Vector3& accel, const AABB& area);
	void SetCamera(Camera* camera);
	void SetBlendMode(BlendMode mode);


	// 更新 / 描画
	void Update(Camera* camera);
	void Draw();
	void Clear();
	void Finalize();

private:
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
		float alpha;  // ★追加：インスタンスごとの透明度
		float pad[3]; // 16バイトアラインメント
	};


	Vector3 scale_ = {1,1,1};
	

	static std::unique_ptr<ParticleManager> instance;
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
	
	std::unordered_map<std::string, ParticleGroup> particleGroups;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	D3D12_VERTEX_BUFFER_VIEW vbView_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;

	

	// ... 既存のメンバ変数の下に追加
	Microsoft::WRL::ComPtr<ID3D12Resource> cbResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vsTransformCB_; // ← 追加: VS用定数バッファ
	void EnsureCapacityBucket(ParticleGroup::BlendBucket& bucket, uint32_t required);
};
