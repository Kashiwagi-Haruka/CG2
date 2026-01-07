#pragma once
#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>

enum BlendMode {

	kBlendModeNone = 0, // ブレンドなし
	kBlendModeAlpha,    // アルファブレンド
	kBlendModeAdd,      // 加算ブレンド
	kBlendModeSub,      // 減算ブレンド
	kBlendModeMultipy,      // 乗算ブレンド
	kBlendScreen,       // スクリーンブレンド
	kCountOfBlendMode   // ブレンドモードの数.使用禁止
};

class BlendModeManager {

	private:



public:
	

	

	D3D12_BLEND_DESC SetBlendMode(BlendMode blendMode);








};
