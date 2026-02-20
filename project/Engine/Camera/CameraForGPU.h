#pragma once
#include "Vector2.h"
#include "Vector3.h"
struct CameraForGpu {
	// カメラのワールド座標（シェーダ側で視点基準計算に使用）
	Vector3 worldPosition;
	// 16byteアラインメント調整用
	float padding = 0.0f;
	// 描画先のスクリーンサイズ（ピクセル）
	Vector2 screenSize;
	// 1 のときグレースケールのフルスクリーンポストエフェクトを適用
	int fullscreenGrayscaleEnabled = 0;
	// 1 のときセピア調のフルスクリーンポストエフェクトを適用
	int fullscreenSepiaEnabled = 0;
	// 定数バッファレイアウト合わせのパディング
	float padding2[2] = {0.0f, 0.0f};
};