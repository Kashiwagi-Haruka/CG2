#pragma once
#include"Wall.h"
#include<vector>
#include"Object3d/Object3d.h"
#include"Light/CommonLight/AreaCommonLight.h"
#include"Primitive/Primitive.h"
#include"GameObject/YoshidaMath/CollisionManager/ObjectCollider.h"

#include<array>
class WallManager
{
public:
    WallManager();
    ~WallManager();
    virtual void Initialize();
    virtual void Update();
    virtual void Draw();
    virtual void SetCamera(Camera* camera);
    std::array<AreaCommonLight,2>& GetAreaLights() { return  areaLights_; }
    std::unique_ptr<Object3d>& GetRoom() {
        return room1_;
    };
    std::unordered_map<std::string, std::unique_ptr<ObjectCollider>>& GetColliders() { return colliders_; }
protected:
    std::unordered_map<std::string, std::unique_ptr<ObjectCollider>>colliders_;
    std::array<AreaCommonLight,2> areaLights_;
    Matrix4x4 roomMat_= { 0.0f };

    std::unique_ptr<Object3d>room1_ = nullptr;
};

