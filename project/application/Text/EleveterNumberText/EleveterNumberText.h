#pragma once
#include "Text/Text.h"

class EleveterNumberText {
public:
	EleveterNumberText();
	void SetStageNumber(int stageNumber);
	void Update();
	void Draw();

private:
	Text text_{};
	uint32_t fontHandle_ = 0;
};