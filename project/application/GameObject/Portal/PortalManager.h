#pragma once
#include"GameObject/WhiteBoard/WhiteBoard.h"
#include"GameObject/Portal/Portal.h"
#include<array>

class TimeCardWatch;
class PlayerCamera;
class PortalManager
{
public:
    PortalManager();
    void Initialize();
    void Update();
    void Draw();
    void SetPlayerCamera(PlayerCamera* camera);
    /// @brief 作成できるポータル地点との当たり判定を作成する
    /// @param timeCardWatch 携帯打刻機
    /// @param camera かめら
    /// @param warpPos ワープ地点の設定をする
    void CheckCollision(TimeCardWatch* timeCardWatch,const Vector3& warpPos);
    std::vector<std::unique_ptr<Portal>>& GetPortals() { return portals_; };
private:
    std::vector<std::unique_ptr<WhiteBoard>> whiteBoards_;
    std::vector<std::unique_ptr<Portal>> portals_;
    PlayerCamera* playerCamera_ = nullptr;


};

