#pragma once
#include"Chair.h"
#include<vector>
class Player;
class ChairManager
{
public:
    static bool IsRayHit() { return isRayHit_; }
    ChairManager();
    ~ChairManager();
    void SetCamera(Camera* camera);
    void Initialize();
    void Update();
    void StandChair(Player* player);
    void Draw();
    void SetPlayerCamera(PlayerCamera* camera);
    std::vector<std::unique_ptr<Chair>>& GetChairs() { return chairs_; };
    static void SetIsStand(const bool isStand) { isStand_ = isStand; };
    static bool GetIsStand() { return isStand_; };
private:
    static bool isStand_;
    static bool isRayHit_;
    std::vector<std::unique_ptr<Chair>>chairs_;
    float standTimer_ = 0.0f;
};

