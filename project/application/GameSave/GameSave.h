#pragma once
#include "Engine/Loadfile/JSON/JsonManager.h"
#include "Transform.h"
#include <string>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

struct PlayerSaveData {
    Transform transform; // プレイヤーの位置、回転、拡縮
};
struct CameraSaveData {
    Transform transform;     // カメラの位置、回転、拡縮

};
struct ProgressSaveData {
    bool isGameClear;             // ゲームクリアしているか
    std::string currentStageName; // 現在のステージ名
    bool isLightHave;             // ライトを持っているか
    bool isKeyHave;//キーを持っているか
};




class GameSave {
public:

private:
    std::string saveDateTime_{};                // セーブデータの保存日時
    PlayerSaveData playerSaveData_{};     // プレイヤーのセーブデータ
    CameraSaveData cameraSaveData_{};     // カメラのセーブデータ
    ProgressSaveData progressSaveData_; // 進行状況のセーブデータ
    bool isInitStart_ = false;
    int selectSlotIndex_ = 0;
private:
    GameSave(); // コンストラクタを private にする
    ~GameSave() {}


public:
    // コピー禁止
    GameSave(const GameSave&) = delete;
    GameSave& operator=(const GameSave&) = delete;

    static GameSave& GetInstance() {
        static GameSave instance;  // C++11以降はスレッドセーフ
        return instance;
    }
    void PlayerSave(const Transform& transform);                                                                    // プレイヤーのセーブデータを保存する
    void CameraSave(const CameraSaveData& saveData); // カメラのセーブデータを保存する
    void ProgressSave(const ProgressSaveData& progressSaveData); // 進行状況のセーブデータを保存する
    //セーブデータをセットする
    void SetSaveDateTime(const std::string& saveDataTime) { saveDateTime_ = saveDataTime; }
    // 引数にスロット番号を追加
    void Save(const int slotIndex);  // セーブデータをファイルに保存する
    void LoadFromIndex(const int slotIndex);  // ファイルからセーブデータを読み込む
    void Load(const std::string& fileName);
    void LoadFirstStage();

    void SetSelectSlotIndex(const int selectSlotIndex) { selectSlotIndex_ = selectSlotIndex; }
    const int GetSelectSlotIndex() { return selectSlotIndex_; }
    void Reset(); // セーブデータをリセットする
    void SetInitStart(const bool flag) { isInitStart_ = flag; }    //最初からスタートするかどうかのフラグをセットする
    bool GetInitStart() { return isInitStart_; }    //最初からスタートするかどうかのフラグを取得する
    const PlayerSaveData& GetPlayerSaveData() { return playerSaveData_; };     // プレイヤーのセーブデータ
    const CameraSaveData& GetCameraSaveData() { return cameraSaveData_; };     // カメラのセーブデータ
    const ProgressSaveData& GetProgressSaveData() { return progressSaveData_; }; // 進行状況のセーブデータ
    const std::string GetCurrentDateTimeString();
    const std::string GetFileName(const int slotIndex);
    const std::string GetScreenShotFileName(const int slotIndex);
    const std::string GetSaveDataTime() {return  saveDateTime_; }



    // ファイルが存在するか確認するヘルパー（オプション）
    bool IsFileExistsAndLoad(const int slotIndex);
};