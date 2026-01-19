#pragma once
#include "Windows.h"
#include"wrl.h"
#include <d3d12.h>
#include "BlendMode/BlendModeManager.h"
class DirectXCommon;
class CreatePSO {


	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_[6];

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_;

	DirectXCommon* dxCommon_ = nullptr;

	HRESULT hr_;

	BlendMode blendMode_;
	BlendModeManager blendModeManager_;

	void CreateRootSignature();
	void CreateGraphicsPipeline(D3D12_CULL_MODE cullMode, bool depthEnable, D3D12_FILL_MODE fillMode, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType);

public:
	CreatePSO(DirectXCommon* dxCommom);
	void Create(
	    D3D12_CULL_MODE cullMode, bool depthEnable = true, D3D12_FILL_MODE fillMode = D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() { return rootSignature_; };
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetGraphicsPipelineState(BlendMode blendMode) { return graphicsPipelineState_[blendMode]; };

};
