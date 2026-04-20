#pragma once
#include "Audio.h"
#include "Drink/Drink.h"
#include "GameObject/YoshidaMath/CollisionManager/Collider.h"
#include "Light/CommonLight/AreaCommonLight.h"
#include "Object3d/Object3d.h"
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include <memory>

class Camera;

class VendingMac : public YoshidaMath::Collider {
public:
	VendingMac();
	~VendingMac();
	/// @brief 衝突時コールバック関数
	void OnCollision(Collider* collider) override;
	/// @brief ワールド座標を取得する
	/// @return ワールド座標
	Vector3 GetWorldPosition() const override;
	const Matrix4x4& GetWorldMatrix() const { return obj_->GetWorldMatrix(); };
	void Update();
	void Initialize();
	void Draw();
	void CheckCollision();
	void SetPlayerCamera(PlayerCamera* camera);
	void SetCamera(Camera* camera);
	AreaCommonLight& GetAreaLight() { return areaLight_; }
	static bool IsRayHit() { return isRayHit_; };
	Vector3 GetForward() const;
	bool ConsumeInteractRequest();

private:
	float GetVol(float length, float maxVol);
	bool OnCollisionRay();
	static bool isRayHit_;
	PlayerCamera* playerCamera_ = nullptr;
	std::unique_ptr<Object3d> obj_ = nullptr;
	std::unique_ptr<Drink> drink_ = nullptr;
	AreaCommonLight areaLight_;
	Vector3 translate_ = {0.0f};
	bool interactRequested_ = false;
};