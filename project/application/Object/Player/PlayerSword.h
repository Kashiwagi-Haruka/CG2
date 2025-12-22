#pragma once
#include "Camera.h"
#include "Object3d.h"
#include "Transform.h"
#include <memory>

class PlayerSword {

	std::unique_ptr<Object3d> swordObject_;
#ifdef _DEBUG
	std::unique_ptr<Object3d> debugBox_;
#endif // _DEBUG
	Camera* camera = nullptr;

	bool isAttacking_ = false;
	float attackTimer_ = 0.0f;
	float attackDuration_ = 0.3f; // 攻撃の持続時間
	int currentComboStep_ = 0;    // 現在のコンボ段階

public:
	PlayerSword();

	void Initialize();
	void Update(const Transform& playerTransform);
	void Draw();

	void StartAttack(int comboStep = 1); // コンボ段階を受け取る
	bool IsAttacking() const { return isAttacking_; }

	Vector3 GetPosition() const;
	float GetHitSize() const { return 1.2f; }
	int GetComboStep() const { return currentComboStep_; }

	void SetCamera(Camera* cam) { camera = cam; }
};