#include "GameSave.h"
void GameSave::CameraSave(const Transform& transform, float rotateSpeed, bool isFlipHorizontally, bool isFlipVertically) {
	cameraSaveData_.transform = transform;
	cameraSaveData_.rotateSpeed = rotateSpeed;
	cameraSaveData_.isFlipHorizontally = isFlipHorizontally;
	cameraSaveData_.isFlipVertically = isFlipVertically;
}

void GameSave::PlayerSave(const Transform& transform) {
	playerSaveData_.transform = transform; }

void GameSave::ProgressSave(bool isGameClear,const std::string& currentStageName,bool isLightHave) {
	progressSaveData_.isGameClear = isGameClear;
	progressSaveData_.currentStageName = currentStageName; 
	progressSaveData_.isLightHave = isLightHave;
}