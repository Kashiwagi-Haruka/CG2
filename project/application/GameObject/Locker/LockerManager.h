#pragma once
#include"Locker.h"
#include<vector>
#include "Sprite.h"

class Player;

class LockerManager
{
public:
    LockerManager(const uint32_t num = 2);
    static bool IsRayHit() { return isRayHit_; }
    void Initialize();
    void Update();
    void InLocker(Player* player);
    void Draw();
    void SetCamera(Camera* camera);
    void SetPlayerCamera(PlayerCamera* playerCamera);
    std::vector<std::unique_ptr<Locker>>& GetLockers() { return lockers_; };
    void DrawSprite();
    static bool GetIsInLocker() { return isInLocker_; }
private:
    std::vector<std::unique_ptr<Locker>>lockers_;
    static bool isRayHit_;
    std::unique_ptr<Sprite> sprite_ = nullptr;
    static  bool isInLocker_;
};

