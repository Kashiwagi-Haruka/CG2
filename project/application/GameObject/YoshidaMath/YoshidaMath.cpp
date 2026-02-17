#include "YoshidaMath.h"
#include"Function.h"
#include<cmath>

float YoshidaMath::Dot(const Vector2& v1, const Vector2& v2)
{
    return { v1.x * v2.x + v1.y * v2.y };
}

float YoshidaMath::Length(const Vector2& v)
{
    return { sqrtf(Dot(v,v)) };
}

Vector2 YoshidaMath::Normalize(const Vector2& v)
{
    float length = Length(v);
    if (length != 0.0f) {
        return { v.x / length,v.y / length };
    } else {
        return { 0.0f, 0.0f };
    }
}

Vector3 YoshidaMath::GetForward(const float angle) {

    Matrix4x4 rotationZMatrix;

    rotationZMatrix = Function::MakeRotateZMatrix(angle);
    Vector3 localForward = { 0.0f, 0.0f, 1.0f };
    return Function::TransformVM(localForward, rotationZMatrix);
}

