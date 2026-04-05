#pragma once
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include <string>
#include <Object3d/Object3d.h>
#include "Primitive/Primitive.h"
#include"Audio.h"
#include"EdamameTrivia.h"
#include"EdamameModel.h"
#include"Light/CommonLight/PointCommonLight.h"

#include<array>
class Edamame
{
public:
    Edamame();
    ~Edamame();
    static bool IsRayHit() { return isRayHit_; }
    void Initialize();
    void Update();
    void Draw();
    void SetPlayerCamera(PlayerCamera* camera);
    void SetCamera(Camera* camera);
    void CheckCollision();
    bool OnCollisionRay();
    void Trivia();
    std::array<PointCommonLight, 2>& GetPointLights() { return pointLights_; };

    std::unique_ptr<EdamameModel>& GetEdamameModel() {return edamameModel_;}
private:
    std::unique_ptr<EdamameModel>edamameModel_ = nullptr;
    std::unique_ptr<EdamameTrivia>edamameTrivia_ = nullptr;
    std::unique_ptr<Object3d>obj_ = nullptr;
    AABB localAABB_ = {};
    PlayerCamera* playerCamera_ = nullptr;
    Transform worldTransform_ = {};
    std::array<PointCommonLight,2> pointLights_;
    static bool isRayHit_;
};

