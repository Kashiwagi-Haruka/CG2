#include "YoshidaMath.h"
#include"Function.h"
#include<cmath>
#include<algorithm>
#include"Camera.h"


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

Matrix4x4 YoshidaMath::MakeRotateMatrix(const Vector3& rotate)
{
    Matrix4x4 rotateX = Function::MakeRotateXMatrix(rotate.x);
    Matrix4x4 rotateY = Function::MakeRotateYMatrix(rotate.y);
    Matrix4x4 rotateZ = Function::MakeRotateZMatrix(rotate.z);

    return  Function::Multiply(Function::Multiply(rotateX, rotateY), rotateZ);
}

Matrix4x4 YoshidaMath::GetBillBordMatrix(Camera* camera)
{
    //ビルボードで作成する
    Matrix4x4 billboardMatrix = camera->GetWorldMatrix();
    billboardMatrix.m[3][0] = 0.0f;
    billboardMatrix.m[3][1] = 0.0f;
    billboardMatrix.m[3][2] = 0.0f;
    return billboardMatrix;
}

Vector3 YoshidaMath::GetAABBScale(const AABB& aabb)
{
    return  {
        aabb.max.x - aabb.min.x,
        aabb.max.y - aabb.min.y,
        aabb.max.z - aabb.min.z
    };
}

