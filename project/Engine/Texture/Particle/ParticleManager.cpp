#include "ParticleManager.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "Logger.h"
#include "SrvManager/SrvManager.h"
#include "TextureManager.h"
#include "VertexData.h"
#include <cassert>
#include <numbers>
std::unique_ptr<ParticleManager> ParticleManager::instance = nullptr;

ParticleManager* ParticleManager::GetInstance() {

	if (instance == nullptr) {
		instance = std::make_unique<ParticleManager>();
	}
	return instance.get();
}

void ParticleManager::SetBlendMode(BlendMode mode) { currentBlendMode_ = mode; }

void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {
	// 1. 記録
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	// 2. ランダム初期化
	srand((unsigned int)time(nullptr));

	// 3. パーティクルPSO
	CreateGraphicsPipeline();

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
	// 4. テクスチャ読み込み(TextureManagerは二重読込防止)
	//--------------------------------
	uint32_t texSrvIndex = TextureManager::GetInstance()->GetTextureIndexByfilePath(textureFilePath);

	//--------------------------------
	// 5. SRVインデックス記録
	//--------------------------------
	newGroup.textureSrvIndex = texSrvIndex;

	//--------------------------------
	// 6. インスタンシング用リソース作成
	//--------------------------------
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

	Matrix4x4 billboard = Function::Inverse(view);
	billboard.m[3][0] = billboard.m[3][1] = billboard.m[3][2] = 0;

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
			// 描画可否チェック
			// ---------------------
			if (!p.visible) {
				++it;
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
			Matrix4x4 c = camera->GetWorldMatrix();
			c.m[3][0] = c.m[3][1] = c.m[3][2] = 0;
			Matrix4x4 world = Function::Multiply(c, Function::MakeAffineMatrix(p.transform_.scale, p.transform_.rotate, p.transform_.translate));
			Matrix4x4 backToFrontMatrix = Function::MakeRotateYMatrix(std::numbers::pi_v<float>);
			Matrix4x4 billboardMatrix = Function::Multiply(billboard, backToFrontMatrix);

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

void ParticleManager::Draw(const std::string& name) {

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

		m->color[0] = 1.0f;
		m->color[1] = 1.0f;
		m->color[2] = 1.0f;
		m->color[3] = 1.0f;

		m->enableLighting = 0;
		m->pad[0] = m->pad[1] = m->pad[2] = 0.0f;
		for (int i = 0; i < 16; i++) {
			m->uvTransform[i] = (i % 5 == 0) ? 1.0f : 0.0f;
		}
		cbResource_->Unmap(0, nullptr);
	}

	// ① RootSig / ② PSO / ③ Heap / ④ CBV(b0) / ⑤ IA 設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
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

	auto it = particleGroups.find(name);
	if (it == particleGroups.end()) {
		return;
	}

	auto& group = it->second;

	// ==========================================
	// ★ 全ブレンドモードをループして描画
	// ==========================================
	for (int blendMode = 0; blendMode < (int)BlendMode::kCountOfBlendMode; blendMode++) {

		// PSO設定
		dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState_[blendMode].Get());

		auto& bucket = group.buckets[blendMode];

		// このブレンドモードで描画するパーティクルがない場合はスキップ
		if (bucket.instanceCount == 0)
			continue;

		// テクスチャ設定
		dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(group.textureSrvIndex));

		// インスタンシングデータ設定
		dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, srvManager_->GetGPUDescriptorHandle(bucket.instancingSrvIndex));

		// 描画
		dxCommon_->GetCommandList()->DrawInstanced(6, bucket.instanceCount, 0, 0);
	}
}
void ParticleManager::Emit(const std::string& name, const Transform& transform, uint32_t count, const Vector3& accel, const AABB& area, float life) {

	auto& group = particleGroups[name];

	for (uint32_t i = 0; i < count; ++i) {

		Particle p{};

		// ★ Emitter の個別範囲でランダム発生
		float rx = area.min.x + (rand() / (float)RAND_MAX) * (area.max.x - area.min.x);
		float ry = area.min.y + (rand() / (float)RAND_MAX) * (area.max.y - area.min.y);
		float rz = area.min.z + (rand() / (float)RAND_MAX) * (area.max.z - area.min.z);

		p.transform_.translate = {transform.translate.x + rx, transform.translate.y + ry, transform.translate.z + rz};

		p.transform_.scale = transform.scale;

		p.life = life;
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

	// 新しいサイズに拡張(2倍ずつ増やす)
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

	// 古いデータをコピー(現存しているインスタンス数だけ)
	uint32_t toCopy = bucket.instanceCount;
	memcpy(newPtr, bucket.instancingDataPtr, sizeof(TransformationMatrix) * toCopy);

	// バッファ差し替え
	bucket.instancingResource = newRes;
	bucket.instancingDataPtr = newPtr;
	bucket.maxInstance = newSize;

	// SRV を更新(同じ SRV index に新バッファを再登録)
	srvManager_->CreateSRVforStructuredBuffer(bucket.instancingSrvIndex, bucket.instancingResource.Get(), bucket.maxInstance, sizeof(TransformationMatrix));
}

void ParticleManager::Finalize() {
	Clear();
	instance = nullptr;
}
void ParticleManager::Clear() {
	for (auto& [name, group] : particleGroups) {
		for (auto& b : group.buckets) {
			if (b.instancingResource) {
				b.instancingResource->Unmap(0, nullptr);
				b.instancingDataPtr = nullptr;
			}
		}
	}
	particleGroups.clear();
}
void ParticleManager::CreateRootsignature() {
	HRESULT hr_;
	// RootParameter 配列を用意 (Material, Texture, Instancing)
	D3D12_ROOT_PARAMETER rootParameters[4] = {};

	// b0 : Material (PS)
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// t0 : Texture (PS)
	D3D12_DESCRIPTOR_RANGE rangeTexture{};
	rangeTexture.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeTexture.NumDescriptors = 1;
	rangeTexture.BaseShaderRegister = 0;
	rangeTexture.RegisterSpace = 0;
	rangeTexture.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[1].DescriptorTable.pDescriptorRanges = &rangeTexture;

	// t1 : InstancingData (VS)
	D3D12_DESCRIPTOR_RANGE rangeInstancing{};
	rangeInstancing.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeInstancing.NumDescriptors = 1;
	rangeInstancing.BaseShaderRegister = 1;
	rangeInstancing.RegisterSpace = 0;
	rangeInstancing.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &rangeInstancing;

	// b1 : Transform (VS)
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[3].Descriptor.ShaderRegister = 1; // b1

	// Sampler s0 : PS
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// RootSignature
	D3D12_ROOT_SIGNATURE_DESC descRootSig{};
	descRootSig.pParameters = rootParameters;
	descRootSig.NumParameters = _countof(rootParameters);
	descRootSig.pStaticSamplers = &sampler;
	descRootSig.NumStaticSamplers = 1;
	descRootSig.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	hr_ = D3D12SerializeRootSignature(&descRootSig, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob_, &errorBlob_);
	if (FAILED(hr_)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}

	hr_ = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob_->GetBufferPointer(), signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr_));
}
void ParticleManager::CreateGraphicsPipeline() {

	CreateRootsignature();
	HRESULT hr_;
	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElements[3] = {};
	inputElements[0] = {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
	inputElements[1] = {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
	inputElements[2] = {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};

	D3D12_INPUT_LAYOUT_DESC inputLayout{};
	inputLayout.pInputElementDescs = inputElements;
	inputLayout.NumElements = _countof(inputElements);

	// DepthStencil
	D3D12_DEPTH_STENCIL_DESC depthDesc{};
	depthDesc.DepthEnable = true;
	depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // ★ 深度書き込みOFF
	depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// Shader
	auto vsBlob = dxCommon_->CompileShader(L"Resources/shader/Particle/Particle.VS.hlsl", L"vs_6_0");
	auto psBlob = dxCommon_->CompileShader(L"Resources/shader/Particle/Particle.PS.hlsl", L"ps_6_0");

	// PSO
	for (int i = 0; i < BlendMode::kCountOfBlendMode; i++) {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
		psoDesc.pRootSignature = rootSignature_.Get();
		psoDesc.InputLayout = inputLayout;
		psoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
		psoDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};
		D3D12_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE; // ★ カリングOFF
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState = rasterizerDesc;
		psoDesc.BlendState = blendModeManager_.SetBlendMode(static_cast<BlendMode>(i));
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		psoDesc.DepthStencilState = depthDesc;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

		hr_ = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&graphicsPipelineState_[i]));
		assert(SUCCEEDED(hr_));
	}
}