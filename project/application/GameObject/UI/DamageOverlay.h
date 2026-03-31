#pragma once
#include "Sprite.h"
#include <memory>

class DamageOverlay {
public:
	DamageOverlay();
	void Initialize();
	void Update(float deltaTime, int hp, int maxHp);
	void Draw() const;
	void StartDisplay();

private:
	void UpdateSpriteColor(int hp, int maxHp);

	std::unique_ptr<Sprite> sprite_ = nullptr;
	float displayTimer_ = 0.0f;
	bool isInitialized_ = false;
	static constexpr float kDisplayDuration_ = 0.3f;
};