#pragma once
#include "WallManager.h"
#include"GameObject/YoshidaMath/CollisionManager/ObjectCollider.h"

class WallManagerRestRoom : public WallManager
{
public:
    WallManagerRestRoom();
    void Update()override;
    void Initialize()override;
    void Draw()override;
    void SetCamera(Camera* camera)override;
    std::unordered_map<std::string, std::unique_ptr<ObjectCollider>>& GetColliders() { return colliders_; }
private:
    std::unordered_map<std::string, std::unique_ptr<ObjectCollider>>colliders_;
};
