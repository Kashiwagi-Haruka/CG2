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
	};
	PlayerSaveData playerSaveData_;
	CameraSaveData cameraSaveData_;


public:
	
	void CameraSave(const Transform& transform, float rotateSpeed, bool isFlipHorizontally, bool isFlipVertically);

	void Save();
	void Load();



};
