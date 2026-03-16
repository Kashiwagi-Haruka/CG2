#pragma once
#include "Transform.h"
#include "Engine/Loadfile/JSON/JsonManager.h"
class GameSave {

	struct PlayerSaveData{
		Transform transform;
	};
	struct CameraSaveData{
		Transform transform;
	};
	struct ProgressSaveData {
	};
	PlayerSaveData playerSaveData_;
	CameraSaveData cameraSaveData_;


public:

	void Save();
	void Load();



};
