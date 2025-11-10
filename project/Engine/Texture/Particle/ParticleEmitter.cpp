#include "ParticleEmitter.h"
#include "ParticleManager.h"

// -----------------------------------------
// コンストラクタ
// -----------------------------------------
ParticleEmitter::ParticleEmitter(const std::string& groupName, const Vector3& pos, float emitFrequency, uint32_t emitCount)
    : name(groupName), position(pos), frequency(emitFrequency), count(emitCount), timer(0.0f) // 明示する
{}

void ParticleEmitter::Update(const Transform& transform) {
	// === 1. 時刻を進める ===
	timer += frequency;

	// === 2. 発生条件チェック ===
	while (timer >= 1.0f) {

		// === 3. Emit（座標は transform.translate を使用）===
		ParticleManager::GetInstance()->Emit(name, transform.translate, count);

		// === 4. 余り時間を保持して頻度計算を正しくする ===
		timer -= 1.0f;
	}
}


// -----------------------------------------
// Emit
// -----------------------------------------
void ParticleEmitter::Emit() {
	// ParticleManagerにEmitを投げるだけ
	ParticleManager::GetInstance()->Emit(name, position, count);
}
