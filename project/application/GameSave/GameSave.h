#pragma once
#include "Engine/Loadfile/JSON/JsonManager.h"
#include "Transform.h"
#include <string>
class GameSave {
	static constexpr const char* kSaveFileName = "gameSave.json";

public:
	struct PlayerSaveData {
		Transform transform; // プレイヤーの位置、回転、スケール
	};
	struct CameraSaveData {
		Transform transform; // カメラの位置、回転、スケール
	};
	struct SceneSaveData {
		std::string sceneName;    // 現在のシーン名
		bool isHaveTimeCardWatch; // タイムカードウォッチの所持状態
	};

	PlayerSaveData playerSaveData_; // プレイヤーのセーブデータ
	CameraSaveData cameraSaveData_; // カメラのセーブデータ
	SceneSaveData sceneSaveData_;   // シーンのセーブデータ

private:
	void ResetToDefault();

public:
	GameSave();

	void SetPlayerSaveData(const PlayerSaveData& data) { playerSaveData_ = data; }
	void SetCameraSaveData(const CameraSaveData& data) { cameraSaveData_ = data; }
	void SetSceneSaveData(const SceneSaveData& data) { sceneSaveData_ = data; }

	const PlayerSaveData& GetPlayerSaveData() const { return playerSaveData_; }
	const CameraSaveData& GetCameraSaveData() const { return cameraSaveData_; }
	const SceneSaveData& GetSceneSaveData() const { return sceneSaveData_; }

	void Save();
	void Load();
};