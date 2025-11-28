#include "ParticleManager.h"
#include "TextureManager.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "VertexData.h"
#include "Camera.h"
#include <numbers>

ParticleManager* ParticleManager::instance = nullptr;


ParticleManager* ParticleManager::GetInstance() {

	if (instance == nullptr) {
		instance = new ParticleManager;
	}
	return instance;
}

void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {
	// 1. 記録
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	// 2. ランダム初期化
	srand((unsigned int)time(nullptr));

	// 3. パーティクルPSOは DirectXCommon::initialize() 内で SetupParticlePSO が実行されている
	//    → ここでは何もする必要なし

	// 4. 頂点データ初期化
	VertexData vertices[6] = {
	    {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-1.0f, 1.0f, 0.0f, 1.0f},  {0.0f, 0.0f}},
        {{1.0f, 1.0f, 0.0f, 1.0f},   {1.0f, 0.0f}},
	    {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{1.0f, 1.0f, 0.0f, 1.0f},   {1.0f, 0.0f}},
        {{1.0f, -1.0f, 0.0f, 1.0f},  {1.0f, 1.0f}}
    };

	// 5. リソース生成
	vertexBuffer_ = dxCommon_->CreateBufferResource(sizeof(vertices));

	// 6. VBView
	vbView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vbView_.StrideInBytes = sizeof(VertexData);
	vbView_.SizeInBytes = sizeof(vertices);

	// 7. データ書き込み
	void* mapped = nullptr;
	vertexBuffer_->Map(0, nullptr, &mapped);
	memcpy(mapped, vertices, sizeof(vertices));
	vertexBuffer_->Unmap(0, nullptr);
}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string& textureFilePath) {
	//--------------------------------
	// 1. 名前重複チェック
	//--------------------------------
	assert(particleGroups.find(name) == particleGroups.end() && "ParticleGroup name already exists!");

	//--------------------------------
	// 2. 新しいパーティクルグループを生成
	//--------------------------------
	ParticleGroup newGroup{};

	//--------------------------------
	// 3. マテリアルデータにテクスチャパス設定
	//--------------------------------
	newGroup.textureFilePath = textureFilePath;

	//--------------------------------
	// 4. テクスチャ読み込み（TextureManagerは二重読込防止）
	//--------------------------------
	uint32_t texSrvIndex = TextureManager::GetInstance()->GetTextureIndexByfilePath(textureFilePath);

	//--------------------------------
	// 5. SRVインデックス記録
	//--------------------------------
	newGroup.textureSrvIndex = texSrvIndex;

	//--------------------------------
	// 6. インスタンシング用リソース作成
	//--------------------------------
	uint32_t maxInstance = 5000; // 初期容量
	uint32_t elementSize = sizeof(TransformationMatrix);
	uint32_t bufferSize = maxInstance * elementSize;

for (int i = 0; i < (int)BlendMode::kCountOfBlendMode; i++) {

		auto& b = newGroup.buckets[i];
		b.maxInstance = 1000;

		uint32_t bufferSize = sizeof(TransformationMatrix) * b.maxInstance;

		b.instancingResource = dxCommon_->CreateBufferResource(bufferSize);
		b.instancingResource->Map(0, nullptr, &b.instancingDataPtr);

		b.instancingSrvIndex = srvManager_->Allocate();
		srvManager_->CreateSRVforStructuredBuffer(b.instancingSrvIndex, b.instancingResource.Get(), b.maxInstance, sizeof(TransformationMatrix));
	}


	//--------------------------------
	// 9. グループ登録
	//--------------------------------
	particleGroups[name] = std::move(newGroup);
}

void ParticleManager::Update(Camera* camera) {

	const Matrix4x4& view = camera->GetViewMatrix();
	const Matrix4x4& proj = camera->GetProjectionMatrix();

	Matrix4x4 camWorld = Function::MakeRotateXMatrix(camera->GetTransform().rotate.x);
	camWorld = Function::Multiply(camWorld, Function::MakeRotateYMatrix(camera->GetTransform().rotate.y));
	camWorld = Function::Multiply(camWorld, Function::MakeRotateZMatrix(camera->GetTransform().rotate.z));

	camWorld.m[3][0] = camWorld.m[3][1] = camWorld.m[3][2] = 0;

	Matrix4x4 billboard = camWorld;

	// ============================
	//  全グループ更新
	// ============================
	for (auto& [name, group] : particleGroups) {

		// ★ 各ブレンドモードの instanceCount をリセット
		for (auto& bucket : group.buckets) {
			bucket.instanceCount = 0;
		}

		for (auto it = group.particles.begin(); it != group.particles.end();) {

			Particle& p = *it;
			

			// ---------------------
			// 寿命
			// ---------------------
			p.life -= 1.0f;
			if (p.life <= 0.0f) {
				it = group.particles.erase(it);
				continue;
			}

			p.color.w -= p.fadeSpeed;
			if (p.color.w <= 0.0f) {
				it = group.particles.erase(it);
				continue;
			}

			// ---------------------
			// 移動
			// ---------------------
			p.vel.x += p.accel.x;
			p.vel.y += p.accel.y;
			p.vel.z += p.accel.z;


			p.transform_.translate.x += p.vel.x;
			p.transform_.translate.y += p.vel.y;
			p.transform_.translate.z += p.vel.z;

			// ---------------------
			// ワールド行列
			// ---------------------
			Matrix4x4 S = Function::MakeScaleMatrix(p.transform_.scale);

			Matrix4x4 R = Function::MakeRotateXMatrix(p.transform_.rotate.x);
			R = Function::Multiply(R, Function::MakeRotateYMatrix(p.transform_.rotate.y));
			R = Function::Multiply(R, Function::MakeRotateZMatrix(p.transform_.rotate.z));

			Matrix4x4 T = Function::MakeTranslateMatrix(p.transform_.translate.x, p.transform_.translate.y, p.transform_.translate.z);

			Matrix4x4 BB = billboard;
			BB.m[3][0] = BB.m[3][1] = BB.m[3][2] = 0;

			Matrix4x4 world = S;
			world = Function::Multiply(R, world);
			world = Function::Multiply(BB, world);
			world = Function::Multiply(T, world);

			Matrix4x4 wvp = Function::Multiply(Function::Multiply(world, view), proj);

			// ================================
			// ブレンドモード別のバケットに追加
			// ================================
			int mode = (int)p.blendmode;
			auto& bucket = group.buckets[mode];

			// 容量不足なら拡張
			if (bucket.instanceCount >= bucket.maxInstance) {

				uint32_t newSize = bucket.maxInstance * 2;
				uint32_t bufferSize = sizeof(TransformationMatrix) * newSize;

				Microsoft::WRL::ComPtr<ID3D12Resource> newRes = dxCommon_->CreateBufferResource(bufferSize);

				void* newPtr = nullptr;
				newRes->Map(0, nullptr, &newPtr);

				memcpy(newPtr, bucket.instancingDataPtr, sizeof(TransformationMatrix) * bucket.instanceCount);

				bucket.instancingResource = newRes;
				bucket.instancingDataPtr = newPtr;
				bucket.maxInstance = newSize;

				srvManager_->CreateSRVforStructuredBuffer(bucket.instancingSrvIndex, bucket.instancingResource.Get(), bucket.maxInstance, sizeof(TransformationMatrix));
			}

			TransformationMatrix* instPtr = reinterpret_cast<TransformationMatrix*>(bucket.instancingDataPtr);

			instPtr[bucket.instanceCount].World = world;
			instPtr[bucket.instanceCount].WVP = wvp;
			instPtr[bucket.instanceCount].alpha = p.color.w;

			bucket.instanceCount++;

			++it;
		}
	}
}



void ParticleManager::Draw() {
	// 必要に応じて CB を作る（1度だけ）
	//if (!cbResource_) {
	//	struct alignas(256) ParticleCB {
	//		float dummy[16];
	//	};
	//	cbResource_ = dxCommon_->CreateBufferResource(sizeof(ParticleCB));
	//}

	//// CB を毎フレーム更新（ここでは内容空）
	//{
	//	void* p = nullptr;
	//	cbResource_->Map(0, nullptr, &p);
	//	memset(p, 0, sizeof(float) * 16);
	//	cbResource_->Unmap(0, nullptr);
	//}
	struct alignas(256) MaterialCB {
	float color[4];     // {1,1,1,1}
	int enableLighting; // 0
	float pad[3];
	float uvTransform[16]; // 単位行列
	};

// 作成は一度だけ
if (!cbResource_)
	cbResource_ = dxCommon_->CreateBufferResource(sizeof(MaterialCB));

// 毎フレーム更新
{
	void* p = nullptr;
	cbResource_->Map(0, nullptr, &p);
	auto* m = reinterpret_cast<MaterialCB*>(p);
	// とりあえず代表粒子の alpha を使う（最小変更）
	
	m->color[0] = 1.0f;
	m->color[1] = 1.0f;
	m->color[2] = 1.0f;
	m->color[3] = 1.0f;

	m->enableLighting = 0;
	m->pad[0] = m->pad[1] = m->pad[2] = 0.0f;
	// 4x4 単位行列
	for (int i = 0; i < 16; i++)
		m->uvTransform[i] = (i % 5 == 0) ? 1.0f : 0.0f;
	cbResource_->Unmap(0, nullptr);
}
// 既に root[0] = b0 にセットしているので、そのままでOK。:contentReference[oaicite:3]{index=3}

	// ① RootSig / ② PSO / ③ Heap / ④ CBV(b0) / ⑤ IA 設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(dxCommon_->GetParticleRootSignature());
	ID3D12DescriptorHeap* heaps[] = {srvManager_->GetDescriptorHeap().Get()};
	dxCommon_->GetCommandList()->SetDescriptorHeaps(1, heaps);
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, cbResource_->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vbView_);

	
	if (!vsTransformCB_) {
		vsTransformCB_ = dxCommon_->CreateBufferResource(sizeof(TransformationMatrix));
		void* p = nullptr;
		vsTransformCB_->Map(0, nullptr, &p);
		auto* t = reinterpret_cast<TransformationMatrix*>(p);
		t->WVP = Function::MakeIdentity4x4();
		t->World = Function::MakeIdentity4x4();
		vsTransformCB_->Unmap(0, nullptr);
	}
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(
	    3, // rootParameters[3] = b1
	    vsTransformCB_->GetGPUVirtualAddress());
	// ==========================================
	// ブレンドモード別に描画
	// ==========================================
	for (int mode = 0; mode < (int)BlendMode::kCountOfBlendMode; mode++) {

		dxCommon_->GetCommandList()->SetPipelineState(dxCommon_->GetParticlePipelineState((BlendMode)mode));

		for (auto& [name, group] : particleGroups) {

			auto& bucket = group.buckets[mode];
			if (bucket.instanceCount == 0)
				continue;

			dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(group.textureSrvIndex));

			dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, srvManager_->GetGPUDescriptorHandle(bucket.instancingSrvIndex));

			dxCommon_->GetCommandList()->DrawInstanced(6, bucket.instanceCount, 0, 0);
		}
	}
}

void ParticleManager::Emit(const std::string& name, const Transform& transform, uint32_t count, const Vector3& accel, const AABB& area) {

	auto& group = particleGroups[name];

	for (uint32_t i = 0; i < count; ++i) {

		Particle p{};

		// ★ Emitter の個別範囲でランダム発生
		float rx = area.min.x + (rand() / (float)RAND_MAX) * (area.max.x - area.min.x);
		float ry = area.min.y + (rand() / (float)RAND_MAX) * (area.max.y - area.min.y);
		float rz = area.min.z + (rand() / (float)RAND_MAX) * (area.max.z - area.min.z);

		p.transform_.translate = {transform.translate.x + rx, transform.translate.y + ry, transform.translate.z + rz};

		p.transform_.scale = transform.scale;

		p.life = 120.0f;
		p.color.w = 1.0f;

		// ★ ここで個別フィールドを保存
		p.accel = accel;
		p.area = area;

		group.particles.push_back(p);
	}
}

void ParticleManager::EnsureCapacityBucket(ParticleGroup::BlendBucket& bucket, uint32_t required) {
	if (required <= bucket.maxInstance)
		return;

	// 新しいサイズに拡張（2倍ずつ増やす）
	uint32_t newSize = bucket.maxInstance;
	while (newSize < required) {
		newSize *= 2;
	}

	uint32_t bufferSize = sizeof(TransformationMatrix) * newSize;

	// 新しいバッファを生成
	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = bufferSize;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	Microsoft::WRL::ComPtr<ID3D12Resource> newRes;
	dxCommon_->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&newRes));

	// 新バッファへマップ
	void* newPtr = nullptr;
	newRes->Map(0, nullptr, &newPtr);

	// 古いデータをコピー（現存しているインスタンス数だけ）
	uint32_t toCopy = bucket.instanceCount;
	memcpy(newPtr, bucket.instancingDataPtr, sizeof(TransformationMatrix) * toCopy);

	// バッファ差し替え
	bucket.instancingResource = newRes;
	bucket.instancingDataPtr = newPtr;
	bucket.maxInstance = newSize;

	// SRV を更新（同じ SRV index に新バッファを再登録）
	srvManager_->CreateSRVforStructuredBuffer(bucket.instancingSrvIndex, bucket.instancingResource.Get(), bucket.maxInstance, sizeof(TransformationMatrix));
}

void ParticleManager::Finalize() {
	for (auto& [name, group] : particleGroups) {
		for (auto& b : group.buckets) {
			if (b.instancingResource) {
				b.instancingResource->Unmap(0, nullptr);
				b.instancingDataPtr = nullptr;
			}
		}
	}
	particleGroups.clear();
	delete instance;
	instance = nullptr;
}