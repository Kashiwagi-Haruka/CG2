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

	PlayerSaveData playerSaveData_;
	CameraSaveData cameraSaveData_;


public:

	void Save();
	void Load();



};
