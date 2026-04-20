#pragma once
#include"WinApp.h"

namespace SCREEN_SIZE {
    const float WIDTH = static_cast<float>(WinApp::kClientWidth);
    const float HEIGHT = static_cast<float>(WinApp::kClientHeight);
    const float HALF_WIDTH = WIDTH * 0.5f;
    const float HALF_HEIGHT = HEIGHT * 0.5f;
}
