#pragma once
#include "Sprite.h"
#include <memory>
class Pause {

	std::unique_ptr<Sprite> BG_;
	std::unique_ptr<Sprite> Select_;
	std::unique_ptr<Sprite> Button_;

	uint32_t BGHandle_;
	uint32_t SelectHandle_;
	uint32_t ButtonHandle_;

	float startTime = 0;

	bool isStart = false;
	bool isSelect = false;

	bool isGameBack = false;
	bool isTitleBack = false;

	public:
	Pause();
	void Initialize();
	void Update(bool isPause);
	void Draw();


};
