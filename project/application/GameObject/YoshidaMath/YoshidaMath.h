#pragma once
#include"Vector2.h"
#include"Vector3.h"
namespace YoshidaMath {
    float Dot(const Vector2& v1, const Vector2& v2);
    float Length(const Vector2& v);
    Vector2 Normalize(const Vector2& v);
    Vector3 GetForward(const float angle);

 
}


