#pragma once
#include<memory>

#include"Text/TextUIManager/TextUIManager.h"
#include "Menu/Menu.h"
#include"GameObject/UI/RaySprite.h"

class UIManager
{
private:
    static bool isPause_;
    std::unique_ptr<TextUIManager> textUIManager_ = nullptr;
    std::unique_ptr<Menu> menu_ = nullptr;
    //raySprite
    std::unique_ptr<RaySprite> raySprite_ = nullptr;
public:
    static bool GetIsPause() { return isPause_; }
    static void SetIsPause(const bool isPause) { isPause_ = isPause; }
    static void TogglePause();
    UIManager();
    void Initialize();
    void Update();
    void CloseOptionANdPrepareResume();
    void CursorShowAndMove();
    void CursorHideAndStop();
    void Draw();
};

