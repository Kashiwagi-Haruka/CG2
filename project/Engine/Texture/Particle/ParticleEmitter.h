#pragma once
#include "Vector3.h"
#include <string>
#include <cstdint>
#include "Transform.h"
class ParticleEmitter {
public:
	// コンストラクタ
	ParticleEmitter(const std::string& groupName, const Vector3& position, float emitFrequency, uint32_t emitCount);

	// 更新
	void Update(const Transform& transform);

	// 発生
	void Emit();




private:
	// メンバ変数（スライド仕様通り全てコンストラクタで設定）
	std::string name;
	Vector3 position;
	float frequency;    // 秒間またはフレームごとの発生頻度
	uint32_t count;     // 一回のEmitで発生する数
	float timer = 0.0f; // 発生管理用のタイマー
};
