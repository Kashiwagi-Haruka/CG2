#include "GameSave.h"
void GameSave::CameraSave(const Transform& transform, float rotateSpeed, bool isFlipHorizontally, bool isFlipVertically) {
	cameraSaveData_.transform = transform;
	cameraSaveData_.rotateSpeed = rotateSpeed;
	cameraSaveData_.isFlipHorizontally = isFlipHorizontally;
	cameraSaveData_.isFlipVertically = isFlipVertically;
}
void GameSave::PlayerSave(const Transform& transform) {
	playerSaveData_.transform = transform; }
void GameSave::ProgressSave(const std::string& currentStageName) {
	progressSaveData_.currentStageName = currentStageName; }