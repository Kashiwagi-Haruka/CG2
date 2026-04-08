#pragma once
#include"Text/Text.h"
#include<array>
#include <memory>
#include"GameObject/YoshidaMath/RandomClass.h"
#include "Audio.h"
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include"GameObject/UI/GameContinued/GameContinued.h"

class GentlemanMenu
{
public:
    enum MenuType { TALK, SAVE,CANCEL, MAX_TEXT};
public:

    //テキストの設定
    GentlemanMenu();
    ~GentlemanMenu();
    void Initialize();
    void Update();
    void Draw();
    static void SetIsShowMenu(const bool isShowMenu) { isShowMenu_ = isShowMenu; }
    static bool GetIsShowMenu() { return isShowMenu_; }
    static uint32_t GetSelectButtonNum() { return selectButtonNum_; }
    static void SetIsSaveMenuShow(const bool isShowSaveMenu) { isShowSaveMenu_ = isShowSaveMenu; }
    static bool GetIsSaveMenuShow() { return isShowSaveMenu_; }

    static void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; };
    static void SetPlayerTransform(Transform* transform) { playerTransform_ = transform; };
    static void SetProgressSaveData(ProgressSaveData* progressSaveData) { progressSaveData_ = progressSaveData; }

    void Save();

private:
    void UpdateGentleManMenu();
private:
    //ゲームコンティニュー
    std::unique_ptr<GameContinued>gameContinued_ = nullptr;

    static PlayerCamera* playerCamera_;
    static Transform* playerTransform_;
    static ProgressSaveData* progressSaveData_;

#pragma region //Text
    uint32_t menuFontHandle_;
    Text triangleText_;
    Text pressEText_;
    std::array<Text, MAX_TEXT> menuText_;
    float fontTheta_ = 0.0f;
#pragma endregion
    static bool isShowMenu_;
    static bool isShowSaveMenu_;
    static uint32_t selectButtonNum_;
    bool isShowStart_ = false;

};

