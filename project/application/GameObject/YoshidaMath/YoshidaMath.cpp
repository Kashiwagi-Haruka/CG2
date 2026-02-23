#include "YoshidaMath.h"
#include"Function.h"
#include<cmath>
#include<algorithm>


float YoshidaMath::Dot(const Vector2& v1, const Vector2& v2)
{
    return { v1.x * v2.x + v1.y * v2.y };
}

float YoshidaMath::Length(const Vector2& v)
{
    return { sqrtf(YoshidaMath::Dot(v,v)) };
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

Vector3 YoshidaMath::GetForward(const Matrix4x4& m) {

    return  Function::Normalize({ m.m[2][0],m.m[2][1], m.m[2][2] });
}


Vector3 YoshidaMath::GetWorldPosByMat(const Matrix4x4& mat)
{
    return { mat.m[2][0], mat.m[2][1], mat.m[2][2] };
}

AABB YoshidaMath::GetAABBWorldPos(const AABB& localAABB, const Vector3& worldPos)
{
    return  { .min = localAABB.min + worldPos,.max = localAABB.max + worldPos };
}

