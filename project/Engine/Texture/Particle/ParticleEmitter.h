#pragma once
#include "Vector3.h"
#include <string>
#include <cstdint>
#include "Transform.h"

class ParticleEmitter {
public:
	// コンストラクタ
	ParticleEmitter(
	    const std::string& groupName, const Transform& transform, float emitFrequency, uint32_t emitCount, Vector3 acceleration = {0, 0, 0}, Vector3 areaMin = {0, 0, 0}, Vector3 areaMax = {1,1,1});

	// 更新
	void Update(const Transform& transform);
	void Draw();
	// 発生
	void Emit();
	void EmitVisible(bool v);

	void SetFrequency(float frequency);
	void SetCount(uint32_t count);
	void SetAcceleration(Vector3 acceleration);
	void SetAreaMin(Vector3 areaMin);
	void SetAreaMax(Vector3 areaMax);
	float GetFrequency() { return frequency; }
	uint32_t GetCount() { return count; }
	Vector3 GetAcceleration() { return acceleration_; }
	Vector3 GetAreaMin() { return areaMin_; }
	Vector3 GetAreaMax() { return areaMax_; }
	// === Transform Getter / Setter ===
	Transform& GetTransformRef() { return transform_; }
	const Transform& GetTransform() const { return transform_; }

		void SetTransform(Transform& t) { transform_ = t; }
	void SetLife(float life) {}
	

private:
	
	// メンバ変数（スライド仕様通り全てコンストラクタで設定）
	std::string name;
	Transform transform_;
	float frequency;    // 秒間またはフレームごとの発生頻度
	uint32_t count;     // 一回のEmitで発生する数
	Vector3 acceleration_; // 加速度
	Vector3 areaMin_;      // 発生エリア最小座標
	Vector3 areaMax_;      // 発生エリア最大座標
	bool emitVisible_ = true;   
	float timer = 0.0f; // 発生管理用のタイマー
	float life = 10;
};
