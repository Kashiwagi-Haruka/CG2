#pragma once
#include"Box.h"
#include<vector>

class BoxManager
{
public:
    BoxManager();
    ~BoxManager();
    void SetCamera(Camera* camera);
    void Initialize();
    void Update();
    void Draw();
    void SetPlayerCamera(PlayerCamera* camera);
    std::vector<std::unique_ptr<Box>>& GetBoxes() { return boxes_; };
private:
    std::vector<std::unique_ptr<Box>>boxes_;
};

