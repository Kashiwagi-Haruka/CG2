#pragma once
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include <string>
#include <Object3d/Object3d.h>
#include "Primitive/Primitive.h"
#include"Audio.h"
#include"EdamameTrivia.h"

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
    void SetModel(const std::string& filePath);
    void CheckCollision();
    bool OnCollisionRay();
    void Trivia();
private:
    std::unique_ptr<EdamameTrivia>edamameTrivia_ = nullptr;
    std::unique_ptr<Object3d>obj_ = nullptr;
    AABB localAABB_ = {};
    PlayerCamera* playerCamera_ = nullptr;
    Transform worldTransform_ = {};
    SoundData edamameBGM_;
    bool isPlaySound_ = false;
};

