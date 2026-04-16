#pragma once
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include "GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"Object3d/Object3d.h"
#include<memory>
#include "Light/CommonLight/AreaCommonLight.h"
#include"Audio.h"
#include "GameObject/VendingMac/VendingDrinkEmitter.h"

class Camera;

class VendingMac : public YoshidaMath::Collider {
public:
    enum class DispenseResult {
        Water,
        Tea,
        EnergyDrink,
        Coffee,
        CoffeeMany,
    };

    VendingMac();
    ~VendingMac();
    /// @brief 衝突時コールバック関数
    void OnCollision(Collider* collider)override;
    /// @brief ワールド座標を取得する
    /// @return ワールド座標
    Vector3 GetWorldPosition() const override;
    const Matrix4x4& GetWorldMatrix() const {return  obj_->GetWorldMatrix(); };
    void Update();
    void Initialize();
    void Draw();
    void CheckCollision();
    void SetPlayerCamera(PlayerCamera* camera);
    void SetCamera(Camera* camera);
    AreaCommonLight& GetAreaLight() { return  areaLight_; }
   static bool IsRayHit() { return isRayHit_; };
  	Vector3 GetForward() const;
	bool ConsumeDispenseResult(DispenseResult& result);
private:
	float GetVol(float length, float maxVol);
	bool OnCollisionRay();
      static bool isRayHit_;
	PlayerCamera* playerCamera_ = nullptr;
	std::unique_ptr<Object3d> obj_ = nullptr;
	AreaCommonLight areaLight_;
	Vector3 translate_ = {0.0f};
	DispenseResult pendingResult_ = DispenseResult::Water;
	bool hasPendingResult_ = false;
	int pressesWithoutCoffeeMany_ = 0;
	std::unique_ptr<VendingDrinkEmitter> drinkEmitter_ = nullptr;
};
