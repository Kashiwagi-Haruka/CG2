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

	// 発生
	void Emit();

private:
	
	// メンバ変数（スライド仕様通り全てコンストラクタで設定）
	std::string name;
	Transform transform_;
	float frequency;    // 秒間またはフレームごとの発生頻度
	uint32_t count;     // 一回のEmitで発生する数
	Vector3 acceleration_; // 加速度
	Vector3 areaMin_;      // 発生エリア最小座標
	Vector3 areaMax_;      // 発生エリア最大座標
	
	float timer = 0.0f; // 発生管理用のタイマー

};
