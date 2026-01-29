#pragma once
#include "Vector4.h"

Vector4 RGBAtoVector4(int r, int g, int b, int a){ return {static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f}; }
