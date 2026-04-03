#pragma once
#include "Transform.h"
#include "Engine/Loadfile/JSON/JsonManager.h"
class GameSave {

	struct PlayerSaveData{
		Transform transform; // プレイヤーの位置、回転、拡縮
	};
	struct CameraSaveData {
		Transform transform;     // カメラの位置、回転、拡縮
		float rotateSpeed;       // カメラの回転速度
		bool isFlipHorizontally; // 左右反転しているか
		bool isFlipVertically;   // 上下反転しているか
	};
	struct ProgressSaveData {
		bool isGameClear;             // ゲームクリアしているか
		std::string currentStageName; // 現在のステージ名
		bool isLightHave; // ライトを持っているか
	};
	PlayerSaveData playerSaveData_; // プレイヤーのセーブデータ
	CameraSaveData cameraSaveData_; // カメラのセーブデータ
	ProgressSaveData progressSaveData_; // 進行状況のセーブデータ

public:
	
	void PlayerSave(const Transform& transform); // プレイヤーのセーブデータを保存する
	void CameraSave(const Transform& transform, float rotateSpeed, bool isFlipHorizontally, bool isFlipVertically); // カメラのセーブデータを保存する
	void ProgressSave(bool isClear, const std::string& currentStageName, bool isLightHave);                         // 進行状況のセーブデータを保存する

	void Save(); // セーブデータをファイルに保存する
	void Load(); // ファイルからセーブデータを読み込む
	void Reset(); // セーブデータをリセットする


};
