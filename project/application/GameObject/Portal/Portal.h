#pragma once
#include"Primitive/Primitive.h"
#include<memory>
#include"Transform.h"
#include"RigidBody.h"
#include"Audio.h"
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"GameObject/WarpPos/WarpPos.h"
#include "RenderTexture2D.h"
#include "Object3d/Object3dCommon.h"
#include <functional>
#include "Mesh/Portal/PortalMesh.h"

class Camera;
class Portal : public YoshidaMath::Collider
{
public:
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const  override;
    void SetTextureIndex(uint32_t index) { portalCircle_->SetTextureIndex(index); };
    RenderTexture2D* GetRenderTexture2D() {
        return portalRenderTexture_.get();
    }
    Portal();
    ~Portal();
    void Initialize();
    void Update();
    void SetCamera(Camera* camera);
    Transform& GetTransform() {return transform_ ;};
    //PortalのSRTをセットする
    void SetParentTransform(Transform* transform) { parentTransform = transform; };
    void SetPortalWorldMatrix();

    Camera* GetCamera() { return warpPos_->GetWarpPosCamera(); };
    //ワープ先を取得する
    WarpPos* GetWarpPos() { return warpPos_.get(); }
    void BeginRender();

    void TransitionToShaderResource();
    void DrawPortals();
    void DrawRings();
    bool GetIsPlayerHit() { return isPlayerHit_; };
private:
    void UpdatePortalWorldMatrix();
    void SetRotateFromDirection(const Vector3& forward);
    void UpdateScale();
    void UpdateWorldMatrix();
    void SetTranslate(const Vector3& forward);
    Vector3 SetSceneCameraAndParentAndGetForward();
    void SetParentTransformToTransform();
private:
    float  preRotY_ = { 0.0f };
    bool isPlayerHit_ = false;
    float scaleTimer_ = 0.0f;
    static Camera* sceneCamera_;

    std::unique_ptr<Primitive>ring_ = nullptr;
    std::unique_ptr<PortalMesh>portalCircle_ = nullptr;
    Vector3 ringTranslate_ = { 0.0f };
    Transform transform_ = {};
    float uvRotateZ_ = 0.0f;
    Sphere sphere_ = { 0.0f };
    //ワープ座標
    std::unique_ptr<WarpPos> warpPos_ = nullptr;
    std::unique_ptr<RenderTexture2D> portalRenderTexture_ = nullptr;
    Transform* parentTransform = nullptr;
};

