#pragma once
#include "Transform.h"
#include "Engine/Loadfile/JSON/JsonManager.h"
class GameSave {

	struct PlayerSaveData{
		Transform transform;
	};
	struct CameraSaveData {
		Transform transform;
		float rotateSpeed;
		bool isFlipHorizontally; // 左右反転しているか
		bool isFlipVertically;   // 上下反転しているか
	};
	struct ProgressSaveData {
		std::string currentStageName;
	};
	PlayerSaveData playerSaveData_;
	CameraSaveData cameraSaveData_;
	ProgressSaveData progressSaveData_;

public:
	
	void PlayerSave(const Transform& transform);
	void CameraSave(const Transform& transform, float rotateSpeed, bool isFlipHorizontally, bool isFlipVertically);
	void ProgressSave(const std::string& currentStageName);

	void Save();
	void Load();



};
