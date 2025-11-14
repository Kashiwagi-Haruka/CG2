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
#include "AABB.h"
struct Particle {
	// 各パーティクルの情報（必要に応じて拡張）
	float pos[3]{};
	float vel[3]{};
	float life{};
};

class SrvManager;
class DirectXCommon;
class Camera;

class ParticleManager {

public:

	struct ParticleGroup {
		std::string textureFilePath; // テクスチャファイル名
		uint32_t textureSrvIndex;    // テクスチャ用 SRV Index（←TextureManagerのsrvIndex）

		std::list<Particle> particles; // パーティクルのリスト

		uint32_t instancingSrvIndex; // インスタンシングデータ用 SRV Index
		Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
		void* instancingDataPtr = nullptr;

		uint32_t instanceCount = 0;

		// 追加: インスタンス最大数
		uint32_t maxInstance = 10000000;
	};

public:
	static ParticleManager* GetInstance();
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);
	void CreateParticleGroup(const std::string& name, const std::string& textureFilePath);
	void Emit(const std::string& name, const Vector3& position, uint32_t count);
	void SetCamera(Camera* camera);
	void SetBlendMode(BlendMode mode);
	//// グループの取得（なければ生成）
	//ParticleGroup& GetGroup(const std::string& groupName);

	// 更新 / 描画
	void Update(Camera* camera);
	void Draw();
	void Finalize();
	
	void SetFieldAcceleration(const Vector3& accel) { accelerationField.Acceleation = accel; }

	void SetFieldArea(const AABB& area) { accelerationField.area = area; }

	const Vector3& GetFieldAcceleration() const { return accelerationField.Acceleation; }

	const AABB& GetFieldArea() const { return accelerationField.area; }

private:
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};
	struct AccelerationField {
		Vector3 Acceleation; // 加速度
		AABB area;         // 範囲
	};
	AccelerationField accelerationField{}; // フィールド

	static ParticleManager* instance;
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
	
	std::unordered_map<std::string, ParticleGroup> particleGroups;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	D3D12_VERTEX_BUFFER_VIEW vbView_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;

	BlendMode blendmode = BlendMode::kBlendModeAlpha;

	// ... 既存のメンバ変数の下に追加
	Microsoft::WRL::ComPtr<ID3D12Resource> cbResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vsTransformCB_; // ← 追加: VS用定数バッファ
	void EnsureCapacity(ParticleGroup& group, uint32_t required);
};
