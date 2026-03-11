#pragma once
#include "GameObject/YoshidaMath/CollisionManager/Collider.h"
#include "Light/SpotLight/SpotLight.h"
#include "Object3d/Object3d.h"
#include <memory>
#include <string>

class Camera;

class Flashlight : public YoshidaMath::Collider {
public:
	Flashlight();
	/// @brief 衝突時コールバック関数
	void OnCollision(Collider* collider) override;
	/// @brief ワールド座標を取得する
	/// @return ワールド座標
	Vector3 GetWorldPosition() const override;
	void SetCamera(Camera* camera);
	void Update();
	void Initialize();
	void Draw();
	void SetLight();
	void ApplyToSpotLight(SpotLight& spotLight, const std::string& name) const;

private:
	Transform transform_ = {};
	std::unique_ptr<Object3d> obj_ = nullptr;
	// SpotLight
	SpotLight spotLight_;
	bool isRotateY_ = false;
};
