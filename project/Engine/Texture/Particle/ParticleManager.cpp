#include "ParticleManager.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "Function.h"
#include "Logger.h"
#include "SrvManager/SrvManager.h"
#include "TextureManager.h"
#include "VertexData.h"
#include <cassert>
#include <cstring>
#include <numbers>

namespace {
Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBufferResource(DirectXCommon* dxCommon, size_t sizeInBytes, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initialState) {
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = flags;

	Microsoft::WRL::ComPtr<ID3D12Resource> bufferResource = nullptr;
	HRESULT hr = dxCommon->GetDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, initialState, nullptr, IID_PPV_ARGS(&bufferResource));
	assert(SUCCEEDED(hr));
	return bufferResource;
}
} // namespace

std::unique_ptr<ParticleManager> ParticleManager::instance = nullptr;

ParticleManager* ParticleManager::GetInstance() {
	if (instance == nullptr) {
		instance = std::make_unique<ParticleManager>();
	}
	return instance.get();
}

void ParticleManager::SetBlendMode(BlendMode mode) { currentBlendMode_ = mode; }

void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	srand((unsigned int)time(nullptr));

	CreateGraphicsPipeline();
	CreateComputePipeline();

	VertexData vertices[6] = {
	    {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-1.0f, 1.0f, 0.0f, 1.0f},  {0.0f, 0.0f}},
        {{1.0f, 1.0f, 0.0f, 1.0f},   {1.0f, 0.0f}},
	    {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{1.0f, 1.0f, 0.0f, 1.0f},   {1.0f, 0.0f}},
        {{1.0f, -1.0f, 0.0f, 1.0f},  {1.0f, 1.0f}}
    };

	vertexBuffer_ = dxCommon_->CreateBufferResource(sizeof(vertices));
	vbView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vbView_.StrideInBytes = sizeof(VertexData);
	vbView_.SizeInBytes = sizeof(vertices);

	void* mapped = nullptr;
	vertexBuffer_->Map(0, nullptr, &mapped);
	memcpy(mapped, vertices, sizeof(vertices));
	vertexBuffer_->Unmap(0, nullptr);

	particleResource_ = CreateDefaultBufferResource(dxCommon_, sizeof(float) * 16 * kMaxParticles_, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
	particleResourceState_ = D3D12_RESOURCE_STATE_COMMON;
	particleSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforStructuredBuffer(particleSrvIndex_, particleResource_.Get(), kMaxParticles_, sizeof(float) * 16);
	particleUavIndex_ = srvManager_->Allocate();
	srvManager_->CreateUAVforStructuredBuffer(particleUavIndex_, particleResource_.Get(), kMaxParticles_, sizeof(float) * 16);
}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string& textureFilePath) {
	assert(particleGroups.find(name) == particleGroups.end() && "ParticleGroup name already exists!");
	ParticleGroup newGroup{};
	newGroup.textureFilePath = textureFilePath;
	newGroup.textureSrvIndex = TextureManager::GetInstance()->GetTextureIndexByfilePath(textureFilePath);
	particleGroups[name] = std::move(newGroup);
}

void ParticleManager::SetCamera(Camera* camera) { camera_ = camera; }

void ParticleManager::Update(Camera* camera) { camera_ = camera; }

void ParticleManager::Draw(const std::string& name) {
	struct alignas(256) MaterialCB {
		float color[4];
		int enableLighting;
		float pad[3];
		float uvTransform[16];
	};

	auto it = particleGroups.find(name);
	if (it == particleGroups.end()) {
		return;
	}

	if (!isParticleInitialized_) {
		InitializeParticlesByCompute();
	}

	if (!cbResource_) {
		cbResource_ = dxCommon_->CreateBufferResource(sizeof(MaterialCB));
	}
	if (!perViewCB_) {
		perViewCB_ = dxCommon_->CreateBufferResource(sizeof(PerView));
	}

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

	PerView perView{};
	if (camera_) {
		const Matrix4x4& view = camera_->GetViewMatrix();
		const Matrix4x4& proj = camera_->GetProjectionMatrix();
		perView.viewProjection = Function::Multiply(view, proj);
		Matrix4x4 billboard = Function::Inverse(view);
		billboard.m[3][0] = billboard.m[3][1] = billboard.m[3][2] = 0.0f;
		perView.billboardMatrix = Function::Multiply(billboard, Function::MakeRotateYMatrix(std::numbers::pi_v<float>));
	} else {
		perView.viewProjection = Function::MakeIdentity4x4();
		perView.billboardMatrix = Function::MakeIdentity4x4();
	}
	{
		void* p = nullptr;
		perViewCB_->Map(0, nullptr, &p);
		memcpy(p, &perView, sizeof(perView));
		perViewCB_->Unmap(0, nullptr);
	}

	if (particleResourceState_ != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = particleResource_.Get();
		barrier.Transition.StateBefore = particleResourceState_;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
		particleResourceState_ = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	}

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	ID3D12DescriptorHeap* heaps[] = {srvManager_->GetDescriptorHeap().Get()};
	dxCommon_->GetCommandList()->SetDescriptorHeaps(1, heaps);
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, cbResource_->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(it->second.textureSrvIndex));
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, srvManager_->GetGPUDescriptorHandle(particleSrvIndex_));
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, perViewCB_->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vbView_);
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState_[(int)currentBlendMode_].Get());
	dxCommon_->GetCommandList()->DrawInstanced(6, kMaxParticles_, 0, 0);
}

void ParticleManager::Emit(const std::string& name, const Transform& transform, uint32_t count, const Vector3& accel, const AABB& area, float life) {
	(void)name;
	(void)transform;
	(void)count;
	(void)accel;
	(void)area;
	(void)life;
}

void ParticleManager::Finalize() {
	Clear();
	instance = nullptr;
}

void ParticleManager::Clear() {
	particleGroups.clear();
	isParticleInitialized_ = false;
}

void ParticleManager::CreateRootsignature() {
	HRESULT hr_;
	D3D12_ROOT_PARAMETER rootParameters[4] = {};

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

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

	D3D12_DESCRIPTOR_RANGE rangeParticle{};
	rangeParticle.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeParticle.NumDescriptors = 1;
	rangeParticle.BaseShaderRegister = 1;
	rangeParticle.RegisterSpace = 0;
	rangeParticle.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &rangeParticle;

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[3].Descriptor.ShaderRegister = 1;

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

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
	D3D12_INPUT_ELEMENT_DESC inputElements[3] = {};
	inputElements[0] = {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
	inputElements[1] = {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
	inputElements[2] = {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};

	D3D12_INPUT_LAYOUT_DESC inputLayout{};
	inputLayout.pInputElementDescs = inputElements;
	inputLayout.NumElements = _countof(inputElements);

	D3D12_DEPTH_STENCIL_DESC depthDesc{};
	depthDesc.DepthEnable = true;
	depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	auto vsBlob = dxCommon_->CompileShader(L"Resources/shader/Particle/Particle.VS.hlsl", L"vs_6_0");
	auto psBlob = dxCommon_->CompileShader(L"Resources/shader/Particle/Particle.PS.hlsl", L"ps_6_0");

	for (int i = 0; i < BlendMode::kCountOfBlendMode; i++) {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
		psoDesc.pRootSignature = rootSignature_.Get();
		psoDesc.InputLayout = inputLayout;
		psoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
		psoDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};
		D3D12_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
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

void ParticleManager::CreateComputePipeline() {
	D3D12_DESCRIPTOR_RANGE uavRange{};
	uavRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	uavRange.NumDescriptors = 1;
	uavRange.BaseShaderRegister = 0;
	uavRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[1] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &uavRange;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	assert(SUCCEEDED(hr));

	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&computeRootSignature_));
	assert(SUCCEEDED(hr));

	auto csBlob = dxCommon_->CompileShader(L"Resources/shader/Particle/Particle.CS.hlsl", L"cs_6_0");
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineDesc{};
	computePipelineDesc.pRootSignature = computeRootSignature_.Get();
	computePipelineDesc.CS = {csBlob->GetBufferPointer(), csBlob->GetBufferSize()};

	hr = dxCommon_->GetDevice()->CreateComputePipelineState(&computePipelineDesc, IID_PPV_ARGS(&computePipelineState_));
	assert(SUCCEEDED(hr));
}

void ParticleManager::InitializeParticlesByCompute() {
	if (isParticleInitialized_) {
		return;
	}

	if (particleResourceState_ != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = particleResource_.Get();
		barrier.Transition.StateBefore = particleResourceState_;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
		particleResourceState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}

	ID3D12DescriptorHeap* heaps[] = {srvManager_->GetDescriptorHeap().Get()};
	dxCommon_->GetCommandList()->SetDescriptorHeaps(1, heaps);
	dxCommon_->GetCommandList()->SetPipelineState(computePipelineState_.Get());
	dxCommon_->GetCommandList()->SetComputeRootSignature(computeRootSignature_.Get());
	dxCommon_->GetCommandList()->SetComputeRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(particleUavIndex_));
	dxCommon_->GetCommandList()->Dispatch(1, 1, 1);

	D3D12_RESOURCE_BARRIER uavBarrier{};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = particleResource_.Get();
	dxCommon_->GetCommandList()->ResourceBarrier(1, &uavBarrier);

	isParticleInitialized_ = true;
}