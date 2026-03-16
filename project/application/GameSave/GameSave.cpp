#include "GameSave.h"
void GameSave::CameraSave(const Transform& transform, float rotateSpeed, bool isFlipHorizontally, bool isFlipVertically) {
	cameraSaveData_.transform = transform;
	cameraSaveData_.rotateSpeed = rotateSpeed;
	cameraSaveData_.isFlipHorizontally = isFlipHorizontally;
	cameraSaveData_.isFlipVertically = isFlipVertically;
}