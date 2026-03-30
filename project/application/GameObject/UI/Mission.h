#pragma once
#include "Text/Text.h"
class Mission {

	uint32_t fontHandle_ = 0;

	Text MissionText_;
	Text KeyGetMissionText_;
	Text GotoDoorMissionText_;

public:
	Mission();
	void Initialize();
	void Update();
	void Draw();

};
