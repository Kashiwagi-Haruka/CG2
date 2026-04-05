#pragma once
#include "Vector2.h"
struct OptionData {

	Vector2 CameraMoveSpeed{1.0f, 1.0f};
	bool isFlipHorizontally = false; // 左右反転しているか
	bool isFlipVertically = false;   // 上下反転しているか

	float BGMVolume = 1.0f;
	float SEVolume = 1.0f;
	float VoiceVolume = 1.0f;
};