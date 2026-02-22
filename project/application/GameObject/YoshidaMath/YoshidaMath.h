#pragma once
#include"Vector2.h"
#include"Vector3.h"
#include"Vector4.h"
#include"RigidBody.h"
#include"Matrix4x4.h"
namespace YoshidaMath {
    float Dot(const Vector2& v1, const Vector2& v2);
    float Length(const Vector2& v);
    Vector2 Normalize(const Vector2& v);
    Vector3 GetForward(const Matrix4x4& m);
    const float kDeltaTime = 1.0f / 60.0f;
    AABB GetAABBWorldPos(const AABB& localAabb,const Vector3& worldPos) ;
}
