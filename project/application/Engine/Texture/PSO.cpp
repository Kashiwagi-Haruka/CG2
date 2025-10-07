#include "PSO.h"
#include <dxcapi.h>
#include <cassert>

//D3D12_GRAPHICS_PIPELINE_STATE_DESC PSO::CreatePSO(BlendMode blendMode, D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc) {
//
//	switch (blendMode) {
//	case PSO::kBlendModeNone:
//		break;
//	case PSO::kBlendModeAlpha:
//		// --- Blend ---
//		D3D12_BLEND_DESC blendDesc{};
//		auto& rtBlend = blendDesc.RenderTarget[0];
//		rtBlend.BlendEnable = FALSE;
//		rtBlend.LogicOpEnable = FALSE;
//		rtBlend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
//		rtBlend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
//		rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
//		rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
//		rtBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
//		rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
//		rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
//		break;
//	case PSO::kBlendModeAdd:
//		break;
//	case PSO::kBlendModeSub:
//		break;
//	case PSO::kBlendModeMul:
//		break;
//	case PSO::kBlendScreen:
//		break;
//	case PSO::kCountOfBlendMode:
//		break;
//	default:
//		break;
//	}
//
//	
//
//	// --- DepthStencil ---
//	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
//	depthStencilDesc.DepthEnable = true;
//	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
//	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
//
//
//
//	// --- 通常PSO（裏面カリング） ---
//	Microsoft::WRL::ComPtr<IDxcBlob> vsBlob = CompileShader(L"Resources/shader/Object3d.VS.hlsl", L"vs_6_0", dxcUtils.Get(), dxcCompiler.Get(), includeHandler.Get());
//	Microsoft::WRL::ComPtr<IDxcBlob> psBlob = CompileShader(L"Resources/shader/Object3d.PS.hlsl", L"ps_6_0", dxcUtils.Get(), dxcCompiler.Get(), includeHandler.Get());
//	assert(vsBlob && psBlob);
//
//	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = baseDesc;
//	psoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
//	psoDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};
//	D3D12_RASTERIZER_DESC rasterizerDesc{};
//	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK /*D3D12_CULL_MODE_NONE*/; // 裏面カリング
//	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
//	psoDesc.RasterizerState = rasterizerDesc;
//
//	return psoDesc;
//}