#pragma once
#include"Desk.h"
#include<vector>
class DeskManager
{
public:
    DeskManager(const uint32_t deskCount = 6);
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
    void SetPlayerCamera(PlayerCamera* playerCamera);
    std::vector<std::unique_ptr<Desk>>& GetDesks() { return desks_; };
    static bool IsRayHit() { return isRayHit_; }
    Matrix4x4* GetDrawerMatrix(const uint32_t deskNum);
private:
    static bool isRayHit_;
    std::vector<std::unique_ptr<Desk>>desks_;
};

