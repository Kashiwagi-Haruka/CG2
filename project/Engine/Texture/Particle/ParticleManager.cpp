#include "ParticleManager.h"
#include "TextureManager.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "VertexData.h"
#include "Camera.h"


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
	uint32_t maxInstance = 1000; // 初期容量
	uint32_t elementSize = sizeof(TransformationMatrix);
	uint32_t bufferSize = maxInstance * elementSize;

	// GPU リソース作成（UPLOAD）
	newGroup.instancingResource = dxCommon_->CreateBufferResource(bufferSize);

	// CPU側から書き込むためにマップ
	newGroup.instancingResource->Map(0, nullptr, &newGroup.instancingDataPtr);

	// 初期値
	newGroup.maxInstance = maxInstance;
	newGroup.instanceCount = 0;

	//--------------------------------
	// 7. SRV を確保してインデックス記録
	//--------------------------------
	newGroup.instancingSrvIndex = srvManager_->Allocate();

	//--------------------------------
	// 8. StructuredBuffer 用 SRV 生成
	//--------------------------------
	srvManager_->CreateSRVforStructuredBuffer(
	    newGroup.instancingSrvIndex,       // 作成先インデックス
	    newGroup.instancingResource.Get(), // 対象リソース
	    maxInstance,                       // 要素数
	    sizeof(TransformationMatrix)       // 1要素のサイズ
	);

	//--------------------------------
	// 9. グループ登録
	//--------------------------------
	particleGroups[name] = std::move(newGroup);
}

void ParticleManager::Update(Camera* camera) {
	// ==== カメラ行列取得 ====
	const Matrix4x4& view = camera->GetViewMatrix();
	const Matrix4x4& proj = camera->GetProjectionMatrix();

	// ==== ビルボード行列計算 ====
	// view の回転成分だけ抜き取り正規化したものを使う
	Matrix4x4 billboard{};
	{
		billboard = view;
		billboard.m[3][0] = 0.0f;
		billboard.m[3][1] = 0.0f;
		billboard.m[3][2] = 0.0f;
		billboard.m[3][3] = 1.0f;
		// 逆行列を使う（回転のみ）
		billboard = Function::Inverse(billboard);
	}

	// ==== 全てのパーティクルグループを処理 ====
	for (auto& [name, group] : particleGroups) {
		// 必要なら容量拡張
		uint32_t required = (uint32_t)group.particles.size();
		EnsureCapacity(group, required);

		// インスタンス書き込み先
		TransformationMatrix* instPtr = reinterpret_cast<TransformationMatrix*>(group.instancingDataPtr);

		uint32_t idx = 0;

		// ==== グループ内の全パーティクルを一重ループで ====
		for (auto it = group.particles.begin(); it != group.particles.end();) {
			Particle& p = *it;

			// --- 寿命処理 ---
			p.life -= 1.0f;
			if (p.life <= 0.0f) {
				it = group.particles.erase(it);
				continue;
			}

			// --- 重力処理 ---
			p.vel[1] -= 0.02f;

			// --- 移動処理 ---
			p.pos[0] += p.vel[0];
			p.pos[1] += p.vel[1];
			p.pos[2] += p.vel[2];

			// --- 経路計算（必要なら後で差し替える場所） ---
			// 例：カーブ、揺れ、スケール変化とか
			// p.pos[0] += sin(p.life * 0.1f) * 0.01f;

			// ---- ワールド行列計算（ビルボード適用）----
			Matrix4x4 translate = Function::MakeTranslateMatrix(p.pos[0], p.pos[1], p.pos[2]);

			Matrix4x4 world = Function::Multiply(billboard, translate); // ビルボードを先に掛ける

			// --- WVP計算 ---
			Matrix4x4 wvp = Function::Multiply(Function::Multiply(world, view), proj);

			// --- インスタンスデータ書き込み ---
			instPtr[idx].World = world;
			instPtr[idx].WVP = wvp;

			idx++;
			++it;
		}

		group.instanceCount = idx;
	}
}


void ParticleManager::SetBlendMode(BlendMode mode) {
	blendmode = mode; }


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
	m->color[0] = m->color[1] = m->color[2] = m->color[3] = 1.0f;
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
	dxCommon_->GetCommandList()->SetPipelineState(dxCommon_->GetParticlePipelineState(blendmode));
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

	// グループごとの描画…
	for (auto& [name, group] : particleGroups) {
		if (group.instanceCount == 0)
			continue;
		dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(group.textureSrvIndex));
		dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, srvManager_->GetGPUDescriptorHandle(group.instancingSrvIndex));
		dxCommon_->GetCommandList()->DrawInstanced(6, group.instanceCount, 0, 0);
	}
}


void ParticleManager::Emit(const std::string& name, const Vector3& position, uint32_t count) {
	//---------------------------------------
	// 1. グループ存在チェック
	//---------------------------------------
	auto it = particleGroups.find(name);
	assert(it != particleGroups.end() && "ParticleGroup name not found!");

	ParticleGroup& group = it->second;

	//---------------------------------------
	// 2. パーティクルを生成して登録
	//---------------------------------------
	for (uint32_t i = 0; i < count; ++i) {
		Particle p{};

		// 初期位置
		p.pos[0] = position.x;
		p.pos[1] = position.y;
		p.pos[2] = position.z;

		// 速度（ランダム：仕様どおり）
		p.vel[0] = (float(rand()) / float(RAND_MAX) - 0.5f) * 0.1f;
		p.vel[1] = (float(rand()) / float(RAND_MAX)) * 0.2f + 0.1f;
		p.vel[2] = (float(rand()) / float(RAND_MAX) - 0.5f) * 0.1f;

		// 寿命（仕様書の指定）
		p.life = 60.0f;

		// グループへ追加
		group.particles.push_back(p);
	}
}

void ParticleManager::EnsureCapacity(ParticleGroup& group, uint32_t required) {
	if (required <= group.maxInstance)
		return;

	uint32_t newSize = group.maxInstance;
	while (newSize < required) {
		newSize *= 2;
	}

	uint32_t bufferSize = sizeof(TransformationMatrix) * newSize;

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

	void* newPtr = nullptr;
	void* oldPtr = group.instancingDataPtr;
	uint32_t oldSize = group.maxInstance;

	// リソース新規作成後
	newRes->Map(0, nullptr, &newPtr);

	// 古いデータをコピー（生存している分だけ）
	uint32_t toCopy = min(oldSize, group.instanceCount);
	memcpy(newPtr, oldPtr, sizeof(TransformationMatrix) * toCopy);

	// 差し替え
	group.instancingResource = newRes;
	group.instancingDataPtr = newPtr;
	group.maxInstance = newSize;


	
	srvManager_->CreateSRVforStructuredBuffer(group.instancingSrvIndex, group.instancingResource.Get(), group.maxInstance, sizeof(TransformationMatrix));
}
void ParticleManager::Finalize() {
	for (auto& [name, group] : particleGroups) {
		if (group.instancingResource) {
			group.instancingResource->Unmap(0, nullptr);
			group.instancingDataPtr = nullptr;
		}
	}
	particleGroups.clear();
	delete instance;
	instance = nullptr;
}