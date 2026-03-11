#pragma once
#include "Transform.h"
#include "Engine/Loadfile/JSON/JsonManager.h"
#include <string>
class GameSave {

	struct PlayerSaveData{
		Transform transform; // プレイヤーの位置、回転、スケール
	};
	struct CameraSaveData{
		Transform transform; // カメラの位置、回転、スケール
	};
	struct SceneSaveData{
		std::string sceneName; // 現在のシーン名
		bool isHaveTimeCardWatch; // タイムカードウォッチの所持状態
	};
	
	PlayerSaveData playerSaveData_; // プレイヤーのセーブデータ
	CameraSaveData cameraSaveData_; // カメラのセーブデータ
	SceneSaveData sceneSaveData_;   // シーンのセーブデータ

public:

	void Save();
	void Load();



};
