#pragma once
#include"Chair.h"
#include<vector>

class ChairManager
{
public:
    ChairManager();
    ~ChairManager();
    void SetCamera(Camera* camera);
    void Initialize();
    void Update();
    void Draw();
    void SetPlayerCamera(PlayerCamera* camera);
    std::vector<std::unique_ptr<Chair>>& GetChairs() { return chairs_; };
private:
    std::vector<std::unique_ptr<Chair>>chairs_;
};

