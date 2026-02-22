#pragma once
#include"WhiteBoard.h"
#include<array>
class Camera;
class WhiteBoardManager
{
public:
    WhiteBoardManager();
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
private:
    std::vector<std::unique_ptr<WhiteBoard>> whiteBoards_;
};

