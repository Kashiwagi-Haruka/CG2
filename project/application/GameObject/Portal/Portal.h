#pragma once
#include"Primitive/Primitive.h"
#include<memory>
#include"Transform.h"
#include"RigidBody.h"
#include"Audio.h"
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"GameObject/WarpPos/WarpPos1.h"
#include "RenderTexture2D.h"
#include "Object3d/Object3dCommon.h"
#include <functional>

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
    void SetCamera(Camera* camera);
    Camera* GetCamera() { return warpPos_->GetWarpPosCamera(); };
    //Portal自身の座標を取得する
    Vector3& GetTranslate() { return transform_.translate; };
    //PortalのSRTをセットする
    void SetParentTransform(Transform* transform) { parentTransform = transform; };
    void SetPortalWorldMatrix();
    //ワープ先の座標を取得する
    Vector3& GetWarpTranslate() { return warpPos_->GetTranslate(); };
    Vector3& GetWarpRotate() { return warpPos_->GetRotate(); };

    //ワープ先の座標をセットする
    void SetWarpTransform(Transform& pos) { warpPos_->SetTransform(pos); };
    const Sphere& GetSphere();
    //ワープ先を取得する
    WarpPos1* GetWarpPos() { return warpPos_.get(); }

    void RenderPortalTextures(const std::function<void(Camera*)>& drawSceneWithoutPortals);
    void UpdateCameraMatrices();
    void DrawPortals();
    void DrawRings();
    void DrawWarpPos();
private:
    float scaleTimer_ = 0.0f;
    void UpdatePortalCamera(const Transform& destinationPortal, Camera* outCamera);
    Camera* sceneCamera_ = nullptr;
    //音楽
    static SoundData warpSE_;
    std::unique_ptr<Primitive>ring_ = nullptr;
    std::unique_ptr<Primitive>portalCircle_ = nullptr;
    Transform transform_ = {};
    float uvRotateZ_ = 0.0f;
    Sphere sphere_ = { 0.0f };
    //ワープ座標
    std::unique_ptr<WarpPos1> warpPos_ = nullptr;
    std::unique_ptr<RenderTexture2D> portalRenderTexture_ = nullptr;
    Transform* parentTransform = nullptr;
};

