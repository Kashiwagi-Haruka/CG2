#pragma once

#include "Text/Text.h"

class Credit {
public:
	void Initialize(uint32_t fontHandle);
	void Draw();

private:
	Text creditBodyText_;
};