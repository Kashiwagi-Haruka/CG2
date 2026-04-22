#pragma once
#include <Transform.h>
#include <string>
#include <Camera.h>
#include <RigidBody.h>
#include <memory>
#include <Object3d/Object3d.h>
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include <GameObject/YoshidaMath/CollisionManager/Collider.h>

class Locker : public YoshidaMath::Collider {
public:
	Locker();
	void Initialize();
	void Update();
	void Draw();
	static void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; };
	void SetCamera(Camera* camera);
	void CheckCollision();
	bool OnCollisionRay();
	/// @brief 衝突時コールバック関数
	void OnCollision(Collider* collider) override;
	/// @brief ワールド座標を取得する
	/// @return ワールド座標
	Vector3 GetWorldPosition() const override;
	void SetEditorRegistrationName(const std::string& name) { editorRegistrationName_ = name; }
	bool IsRayHit() { return isRayHit_; }
	const Matrix4x4& GetWorldMatrix() const { return obj_->GetWorldMatrix(); };

private:
	bool isRayHit_ = false;
	static PlayerCamera* playerCamera_;
	std::unique_ptr<Object3d> obj_ = nullptr;
	std::string editorRegistrationName_ = "Locker";
};
