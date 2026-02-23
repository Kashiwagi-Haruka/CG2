#pragma once
#include"Vector2.h"
#include"Vector3.h"
#include"Vector4.h"
#include"RigidBody.h"
#include"Matrix4x4.h"
class Collider;
namespace YoshidaMath {
    const constexpr float kDeltaTime = 1.0f / 60.0f;
    const constexpr float kGravity = 0.98f;

    float Dot(const Vector2& v1, const Vector2& v2);
    float Length(const Vector2& v);
    Vector2 Normalize(const Vector2& v);
    //行列から前方のベクトルを取得する
    Vector3 GetForward(const Matrix4x4& m);
    //ワールド行列からワールド座標を取得する
    Vector3 GetWorldPosByMat(const Matrix4x4& mat);
    AABB GetAABBWorldPos(const AABB& localAABB, const Vector3& worldPos);
}
