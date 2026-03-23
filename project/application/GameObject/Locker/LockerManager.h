#pragma once
#include"Locker.h"
#include<vector>
class LockerManager
{
public:
    LockerManager();
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
    void SetPlayerCamera(PlayerCamera* playerCamera);
    std::vector<std::unique_ptr<Locker>>& GetLockers() { return lockers_; };
private:
    static constexpr uint32_t kMaxLockers_ = 2;
    std::vector<std::unique_ptr<Locker>>lockers_;
};

