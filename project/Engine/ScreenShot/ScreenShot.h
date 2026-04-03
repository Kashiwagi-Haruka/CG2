#pragma once

#include <string>

class DirectXCommon;

class ScreenShot {
public:
	// バックバッファを画像として保存する
	// filePath は UTF-8 文字列で指定可能（PNGとして保存）
	static bool SaveBackBuffer(DirectXCommon* dxCommon, const std::string& filePath);
};