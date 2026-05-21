#pragma once
#include <memory>
#include "Mesh/Object3d/Object3d.h"
#include "Transform.h"
#include <GameObject/YoshidaMath/CollisionManager/Collider.h>
class Camera;
class Enemy : public YoshidaMath::Collider {

	std::unique_ptr<Object3d> obj_ = nullptr;
	Transform transform_;

	enum class EnemyState {
		Idle,
		Walking,
		Break,
	} state_ = EnemyState::Idle;


public:
	Enemy();
	void Initialize();
	void SetCamera(Camera* camera);
	void Update();
	void Draw();
	/// @brief 衝突時コールバック関数
	void OnCollision(Collider* collider) override;
	/// @brief ワールド座標を取得する
	/// @return ワールド座標
	Vector3 GetWorldPosition() const override;
	const Matrix4x4& GetWorldMatrix() const override;

};
