#include "GameTimer.h"
std::unique_ptr<GameTimer> GameTimer::instance = nullptr;

GameTimer* GameTimer::GetInstance() {

	if (instance == nullptr) {
		instance = std::make_unique<GameTimer>();
	}
	return instance.get();
}
void GameTimer::Reset(){ 
	timer_ = 0.0f; 
}

void GameTimer::Update(){ timer_ += 1.0f / 60.0f; }