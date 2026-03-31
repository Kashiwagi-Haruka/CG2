#pragma once
#include"Desk.h"
#include<vector>
class DeskManager
{
public:
    DeskManager();
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
    void SetPlayerCamera(PlayerCamera* playerCamera);
    std::vector<std::unique_ptr<Desk>>& GetDesks() { return desks_; };
    static bool IsRayHit() { return isRayHit_; }
private:
    static bool isRayHit_;
    static constexpr uint32_t kMaxDesks_ = 6;
    std::vector<std::unique_ptr<Desk>>desks_;
};

