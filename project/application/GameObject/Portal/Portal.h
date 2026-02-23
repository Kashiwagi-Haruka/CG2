#pragma once
#include"Primitive/Primitive.h"
#include<memory>
#include"Transform.h"
#include"RigidBody.h"
#include"Audio.h"
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"

class Camera;
class Portal : public YoshidaMath::Collider
{
public:
    //音の設定
    static void LoadSE();
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const  override;

    Portal();
    ~Portal();
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
    Vector3& GetTranslate() { return transform_.translate; };
    const Sphere& GetSphere();
private:
    //音楽
    static SoundData warpSE_;
    std::unique_ptr<Primitive>ring_ = nullptr;
    Transform transform_ = {};
    Matrix4x4 uvMat_ = { 0.0f };
    float uvTranslateY_ = 0.0f;
    Sphere sphere_ = { 0.0f };
};

