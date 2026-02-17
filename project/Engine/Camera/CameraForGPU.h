#pragma once
#include "Vector2.h"
#include "Vector3.h"
struct CameraForGpu {
	Vector3 worldPosition;
	float padding = 0.0f;
	Vector2 screenSize;
	int fullscreenGrayscaleEnabled = 0;
	float padding2 = 0.0f;
};