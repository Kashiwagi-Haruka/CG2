#pragma once
#include "Object3d/InstancedObject3d/InstancedObject3d.h"
#include <memory>
#include <vector>
#include "Vector3.h"

class Camera;

class DocumentParticle {
public:
	DocumentParticle();
	~DocumentParticle() = default;
	void Initialize();
	void Update(Camera* camera, const Vector3& lightDirection);
	void Draw();
	void Emit(const Vector3& center, uint32_t count);

	// --- 時間発生用の追加関数 ---
	// 時間経過による連続発生を開始する
	// interval: 何秒ごとに発生させるか / countPerEmit: 1回に何枚発生させるか
	void StartEmit(Vector3* center, float interval, uint32_t countPerEmit);
	// 連続発生を停止する
	void StopEmit();
	void SetTexture();
	// ★ 追加：発生範囲（ランダムで散らばる幅）を設定する関数
	void SetEmitArea(const Vector3& areaMin, const Vector3& areaMax);
private:

	struct DocumentData {
		Vector3 position;
		Vector3 velocity;
		Vector3 rotation;
		Vector3 rotationVelocity;
		float wobblePhase;
		float lifeTime;
		float maxLife;
		bool isActive = false;
	};

	std::unique_ptr<InstancedObject3d> instancedObject_ = nullptr;
	std::vector<DocumentData> instances_{};
	uint32_t activeInstanceCount_ = 0;
	// 最大発生数
	static constexpr uint32_t kMaxInstanceCount = 100;

	// --- 時間発生用のメンバ変数 ---
	bool isEmitting_ = false;       // 発生中かどうか
	Vector3* emitCenter_ = nullptr; // 発生位置
	float emitTimer_ = 0.0f;        // 経過時間計測用
	float emitInterval_ = 0.1f;     // 発生間隔（秒）
	uint32_t emitCount_ = 1;        // 1回あたりの発生数

	// ★ 追加：発生範囲の最小値と最大値（デフォルトはズレなし）
	Vector3 emitAreaMin_ = { 0.0f, 0.0f, 0.0f };
	Vector3 emitAreaMax_ = { 0.0f, 0.0f, 0.0f };
};