#pragma once
#include"Chair.h"
#include<vector>

class ChairManager
{
public:
    static bool IsRayHit() { return isRayHit_; }
    ChairManager();
    ~ChairManager();
    void SetCamera(Camera* camera);
    void Initialize();
    void Update();
    void Draw();
    void SetPlayerCamera(PlayerCamera* camera);
    std::vector<std::unique_ptr<Chair>>& GetChairs() { return chairs_; };
private:
    static bool isRayHit_;
    std::vector<std::unique_ptr<Chair>>chairs_;
};

