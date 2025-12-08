#include "BulletManager.h"
#include "ModelManeger.h"
#include "Function.h"
#include "Camera.h"

void BulletManager::Initialize(Camera* camera) {
	camera_ = camera;
	ModelManeger::GetInstance()->LoadModel("playerBullet");


	line_.SetCamera(camera_);
	line_.Initialize();
}

void BulletManager::SetPlayerPos(const Vector3& pos) { line_.SetPlayerPos(pos); }

void BulletManager::SetShotDir(const Vector3& dir) { line_.SetShotDir(dir); }

void BulletManager::Fire(const Vector3& pos, const Vector3& dir) {
	auto bullet = std::make_unique<PlayerBullet>();
	bullet->Initialize(camera_);
	bullet->SetVelocity(dir * 0.5f);
	bullet->Charge(pos, dir);
	bullet->Fire();

	bullets_.push_back(std::move(bullet));
}

bool BulletManager::Collision(Vector3 ePos) {
	bool isHit = false;
	float bulletHitSize = 1.0f;

	bullets_.erase(
	    std::remove_if(
	        bullets_.begin(), bullets_.end(),
	        [&](const std::unique_ptr<PlayerBullet>& b) {
		        Vector3 p = b->GetPosition();
		        bool hit = fabs(p.x - ePos.x) < bulletHitSize && fabs(p.y - ePos.y) < bulletHitSize;

		        if (hit)
			        isHit = true; // ★ 一つでも当たれば true

		        return hit; // ★ hit した弾だけ削除される
	        }),
	    bullets_.end());

	return isHit;
}

void BulletManager::Update() {
	// ---- 弾更新 ----
	for (auto& b : bullets_) {
		b->Update(camera_);
	}

	// ここで死んだ弾を消すなら erase(remove_if) を入れる

	// ---- 予測線更新 ----
	line_.Update();
}

void BulletManager::Draw() {
	// 弾描画
	for (auto& b : bullets_) {
		b->Draw();
	}

	// ライン描画（1本だけ）
	line_.Draw();
}
void BulletManager::SetCamera(Camera* camera) {
	camera_ = camera;
	line_.SetCamera(camera); // ← 予測線にもカメラ更新
	// 弾にもカメラを反映したいなら以下追加：

}
