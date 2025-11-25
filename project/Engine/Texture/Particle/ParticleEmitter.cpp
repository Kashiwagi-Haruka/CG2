#include "ParticleEmitter.h"
#include "ParticleManager.h"
#include "Function.h"
// -----------------------------------------
// コンストラクタ
// -----------------------------------------
ParticleEmitter::ParticleEmitter(
    const std::string& groupName, const Transform& transform, float emitFrequency, uint32_t emitCount, Vector3 acceleration, Vector3 areaMin, Vector3 areaMax)
    : name(groupName), transform_(transform), frequency(emitFrequency), count(emitCount), acceleration_(acceleration),areaMin_(areaMin),areaMax_(areaMax), timer(0.0f) // 明示する
{}

void ParticleEmitter::Update(const Transform& transform) {
	// === 1. 時刻を進める ===
	timer += frequency;
	transform_.translate = transform.translate;
	// === 2. 発生条件チェック ===
	while (timer >= 1.0f) {
		ParticleManager::GetInstance()->SetFieldAcceleration(acceleration_); 
		ParticleManager::GetInstance()->SetFieldArea(AABB(transform_.translate-areaMin_, transform_.translate+areaMax_));
		
		// === 3. Emit（座標は transform.translate を使用）===
		ParticleManager::GetInstance()->Emit(name, transform_, count);

		// === 4. 余り時間を保持して頻度計算を正しくする ===
		timer -= 1.0f;
	}
}


// -----------------------------------------
// Emit
// -----------------------------------------
void ParticleEmitter::Emit() {
	// ParticleManagerにEmitを投げるだけ
	ParticleManager::GetInstance()->Emit(name, transform_, count);
}
