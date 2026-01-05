#pragma once
#include <memory>
class GameTimer {

	float timer_=0.0f;
	static std::unique_ptr<GameTimer> instance;
	public:

	static GameTimer* GetInstance();
	void Reset();
	void Update();
	float GetTimer() { return timer_; };


};
