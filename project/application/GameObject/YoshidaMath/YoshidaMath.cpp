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


AABB YoshidaMath::GetAABBWorldPos(const AABB& localAABB, const Vector3& worldPos)
{
    return  { .min = localAABB.min + worldPos,.max = localAABB.max + worldPos };
}

bool YoshidaMath::RayIntersectsAABB(const Ray& ray, const AABB& box, float& tMin, float& tMax) {

    tMin = 0.0f;
    tMax = std::numeric_limits<float>::max();

    float minVal = 0.0f;
    float maxVal = 0.0f;
    float origin = 0.0f;
    float dir = 0.0f;

    for (int i = 0; i < 3; i++) {

        if (i == 0) {
            minVal = box.min.x;
            maxVal = box.max.x;
            origin = ray.origin.x;
            dir = ray.diff.x;
        }
        if (i == 1) {
            minVal = box.min.y;
            maxVal = box.max.y;
            origin = ray.origin.y;
            dir = ray.diff.y;
        }
        if (i == 2) {
            minVal = box.min.z;
            maxVal = box.max.z;
            origin = ray.origin.z;
            dir = ray.diff.z;
        }

        if (std::abs(dir) < 1e-6f) {
            // レイが軸に平行
            if (origin < minVal || origin > maxVal) {
                return false;
            }
        } else {
            float t1 = (minVal - origin) / dir;
            float t2 = (maxVal - origin) / dir;

            if (t1 > t2) std::swap(t1, t2);

            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);

            if (tMin > tMax) {
                return false;
            }
        }
    }

    return true;
}

Vector3 YoshidaMath::GetWorldPosByMat(const Matrix4x4& mat)
{
    return { mat.m[2][0], mat.m[2][1], mat.m[2][2] };
}
