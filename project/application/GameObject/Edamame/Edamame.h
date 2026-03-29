#pragma once
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include <string>
#include <Object3d/Object3d.h>
#include "Primitive/Primitive.h"
#include"Audio.h"
#include"EdamameTrivia.h"
#include"EdamameModel.h"
#include"Light/SpotLight.h"


class Edamame
{
public:
    Edamame();
    ~Edamame();
    void Initialize();
    void Update();
    void Draw();
    void SetPlayerCamera(PlayerCamera* camera);
    void SetCamera(Camera* camera);
    void CheckCollision();
    bool OnCollisionRay();
    void Trivia();
    SpotLight& GetSpotLight() { return spotLight_; };
    std::unique_ptr<EdamameModel>& GetEdamameModel() {return edamameModel_;}
private:
    std::unique_ptr<EdamameModel>edamameModel_ = nullptr;
    std::unique_ptr<EdamameTrivia>edamameTrivia_ = nullptr;
    std::unique_ptr<Object3d>obj_ = nullptr;
    AABB localAABB_ = {};
    PlayerCamera* playerCamera_ = nullptr;
    Transform worldTransform_ = {};
    SpotLight spotLight_;
};

