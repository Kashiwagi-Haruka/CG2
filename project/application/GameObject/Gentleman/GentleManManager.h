#pragma once

#include <memory>

#include "Gentleman.h"


class PlayerCamera;
class Camera;
class GentleManManager
{
private:
    //セーブポイント紳士
    std::unique_ptr<Gentleman> gentleman_ = nullptr;

    Player* player_ = nullptr;
#pragma region // セーブ
    bool isTired_ = false;
    bool isSit_ = false;
public:
    Gentleman* GetGentleman() { return gentleman_.get(); }
    GentleManManager();
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
    void SetPlayerCamera(PlayerCamera* playerCamera);
    void SetPlayer(Player* player);
};

