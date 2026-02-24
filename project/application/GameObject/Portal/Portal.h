#pragma once
#include"Primitive/Primitive.h"
#include<memory>
#include"Transform.h"
#include"RigidBody.h"
#include"Audio.h"
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"GameObject/WarpPos/WarpPos1.h"

class Camera;
class Portal : public YoshidaMath::Collider
{
public:
    //音の設定
    static void LoadSE();
    static void UnLoadSE();
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
    //Portal自身の座標を取得する
    Vector3& GetTranslate() { return transform_.translate; };
    //PortalのSRTをセットする
    void SetTransform(const Transform& transform) { transform_ = transform; };
    void SetRingWorldMatrix(Camera* camera);
    //ワープ先の座標を取得する
    Vector3& GetWarpPos() { return warpPos_->GetTranslate(); };
    //ワープ先の座標をセットする
    void SetWarpPos(const Vector3& pos) { warpPos_->SetTransform(pos); };
    const Sphere& GetSphere();
private:

    //音楽
    static SoundData warpSE_;
    std::unique_ptr<Primitive>ring_ = nullptr;
    Transform transform_ = {};
    Matrix4x4 uvMat_ = { 0.0f };
    float uvTranslateY_ = 0.0f;
    Sphere sphere_ = { 0.0f };
    //ワープ座標
    std::unique_ptr<WarpPos1> warpPos_ = nullptr;
};

