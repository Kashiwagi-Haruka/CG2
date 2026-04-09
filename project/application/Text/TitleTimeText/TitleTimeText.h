#pragma once

#include "Text/Text.h"

class TitleTimeText
{
private:
    uint32_t fontHandle_;
    Text dateText_;
	std::string preCurrentDataTime_;
public:
	TitleTimeText();
	void Initialize();
	void Update();
	void Draw();
};

