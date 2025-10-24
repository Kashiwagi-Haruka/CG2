#pragma once
#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

struct VertexData {
	Vector4 position; // 頂点の位置（x, y, z, w）
	Vector2 texcoord; // テクスチャ座標（u, v）
	Vector3 normal;
};
