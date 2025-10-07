#pragma once
#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
class PSO {

public:
	enum BlendMode {

		kBlendModeNone = 0, // ブレンドなし
		kBlendModeAlpha,    // アルファブレンド
		kBlendModeAdd,      // 加算ブレンド
		kBlendModeSub,      // 減算ブレンド
		kBlendModeMul,      // 乗算ブレンド
		kBlendScreen,       // スクリーンブレンド
		kCountOfBlendMode   // ブレンドモードの数.使用禁止
	};

	

	D3D12_GRAPHICS_PIPELINE_STATE_DESC CreatePSO(BlendMode blendMode, D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc);








};
