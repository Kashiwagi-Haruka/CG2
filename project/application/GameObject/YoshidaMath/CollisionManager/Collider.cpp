#define NOMINMAX
#include "Collider.h"
#include"Function.h"
#include "Object3d/Object3dCommon.h"
YoshidaMath::Collider::Collider()
{
    collisionInfo_.collided = false;
    collisionInfo_.normal = { 0.0f,0.0f,0.0f };
    collisionInfo_.penetration = { 0.0f };
}


void YoshidaMath::Collider::SetRadius(float radius)
{
    type_ = ColliderType::kSphere;
    radius_ = radius;
}


void YoshidaMath::Collider::SetAABB(const AABB& aabb, const bool isRound) {

    if (isRound) {
        type_ = ColliderType::kOBB;
    } else {
        type_ = ColliderType::kAABB;
    }

    AABB_ = aabb;

};

bool YoshidaMath::RayIntersectsAABB(const Ray& ray, const AABB& box, float tMin, float tMax) {

    //tMin = 0.0f;
    //tMax = std::numeric_limits<float>::max();

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


YoshidaMath::CollisionInfo YoshidaMath::GetCollisionInfo(const AABB& a, const AABB& b) {

    CollisionInfo result;

    if (!IsCollision(a, b)) {
        result.collided = false;
        return result;
    }

    result.collided = true;
    //オーバーラップを調べる
    float overlapX = std::min(a.max.x - b.min.x, b.max.x - a.min.x);
    float overlapY = std::min(a.max.y - b.min.y, b.max.y - a.min.y);
    float overlapZ = std::min(a.max.z - b.min.z, b.max.z - a.min.z);

    Vector3 centerA = GetAABBCenter(a);
    Vector3 centerB = GetAABBCenter(b);

    //最小のオーバーラップ軸を分離する
    if (overlapX <= overlapY && overlapX <= overlapZ) {

        result.penetration = overlapX;
        result.normal = (centerA.x < centerB.x) ? Vector3(-1.0f, 0.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);

    } else if (overlapY <= overlapZ) {
        result.penetration = overlapY;
        result.normal = (centerA.y < centerB.y) ? Vector3(0.0f, -1.0f, 0.0f) : Vector3(0.0f, 1.0f, 0.0f);
    } else {
        result.penetration = overlapZ;
        result.normal = (centerA.z < centerB.z) ? Vector3(0.0f, 0.0f, -1.0f) : Vector3(0.0f, 0.0f, 1.0f);
    }

    return result;
}

void YoshidaMath::ResolveCollision(Vector3& pos, Vector3& velocity, const YoshidaMath::CollisionInfo& info) {

    if (!info.collided) return;

    pos += info.normal * info.penetration;

    float normalVelocity = Function::Dot(velocity, info.normal);

    if (normalVelocity < 0.0f) {
        velocity.x -= info.normal.x * normalVelocity;
        velocity.y -= info.normal.y * normalVelocity;
        velocity.z -= info.normal.z * normalVelocity;
    }
}
Sphere YoshidaMath::GetSphereWorldPos(YoshidaMath::Collider* sphere)
{
    return Sphere{
 .center = sphere->GetWorldPosition(),
 .radius = sphere->GetRadius()
    };
}

bool YoshidaMath::IsCollision(const Sphere& s1, const Sphere& s2)
{

    //2つの急の中心点間距離を求める 
    float distance = Function::Length({ s2.center - s1.center });

    if (distance <= s1.radius + s2.radius) {
        return true;
    }

    return false;
}

bool YoshidaMath::IsCollision(const AABB& a, const AABB& b)
{

    if ((a.min.x <= b.max.x && a.max.x >= b.min.x) &&//x軸
        (a.min.y <= b.max.y && a.max.y >= b.min.y) &&//y軸
        (a.min.z <= b.max.z && a.max.z >= b.min.z)) {
        return true;
    }

    return false;
}

void YoshidaMath::UpdateOBB(YoshidaMath::Collider* obb)
{    // 中心位置
    auto& mat = obb->GetWorldMatrix();
    auto& newObb = obb->GetOBB();
    newObb.center = YoshidaMath::GetWorldPosByMat(mat);

    // 半径（scale の半分）
    Vector3 scale = YoshidaMath::GetAABBScale(obb->GetAABB());
    newObb.halfSize = scale * 0.5f;

    // 回転行列から軸を取り出す
    newObb.axis[0] = { mat.m[0][0],mat.m[1][0], mat.m[2][0] }; // X軸
    newObb.axis[1] = { mat.m[0][1],mat.m[1][1], mat.m[2][1] }; // Y軸
    newObb.axis[2] = { mat.m[0][2],mat.m[1][2], mat.m[2][2] }; // Z軸

    // 正規化
    for (int i = 0; i < 3; i++) {
        newObb.axis[i] = Function::Normalize(newObb.axis[i]);
    }
};

AABB YoshidaMath::GetAABBWorldPos(YoshidaMath::Collider* aabb)
{
    AABB aabbWorld = aabb->GetAABB();
    Vector3 pos = aabb->GetWorldPosition();
    aabbWorld.min = aabbWorld.min + pos;
    aabbWorld.max = aabbWorld.max + pos;
    return aabbWorld;

}

Vector3 YoshidaMath::GetAABBCenter(const AABB& aabb)
{
    return (aabb.min + aabb.max) * 0.5f;
}


bool YoshidaMath::IsCollision(const AABB& aabb, const OBB& obb)
{
    // AABB の中心と半径
    Vector3 aCenter = (aabb.min + aabb.max) * 0.5f;
    Vector3 aHalf = (aabb.max - aabb.min) * 0.5f;

    // AABB の軸（ワールド軸）
    Vector3 aAxis[3] = {
        {1,0,0},
        {0,1,0},
        {0,0,1}
    };

    // OBB の中心
    Vector3 tWorld = obb.center - aCenter;

    // AABB のローカル座標系に変換
    float t[3] = {
        Function::Dot(tWorld, aAxis[0]),
        Function::Dot(tWorld, aAxis[1]),
        Function::Dot(tWorld, aAxis[2])
    };

    // AbsR[i][j] = |Ai・Bj|
    float AbsR[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            AbsR[i][j] = fabs(Function::Dot(aAxis[i], obb.axis[j])) + 1e-5f;
        }
    }

    float ra, rb;

    // =========
    // 1. AABB の軸（3本）
    // =========
    for (int i = 0; i < 3; i++) {

        if (i == 0) {
            ra = aHalf.x;
        } else if (i == 1) {
            ra = aHalf.y;
        } else {
            ra = aHalf.z;
        }

        rb = obb.halfSize.x * AbsR[i][0] +
            obb.halfSize.y * AbsR[i][1] +
            obb.halfSize.z * AbsR[i][2];

        if (fabs(t[i]) > ra + rb) return false;
    }

    // =========
    // 2. OBB の軸（3本）
    // =========
    for (int i = 0; i < 3; i++) {
        ra = aHalf.x * AbsR[0][i] +
            aHalf.y * AbsR[1][i] +
            aHalf.z * AbsR[2][i];
        if (i == 0) {
            rb = obb.halfSize.x;
        } else if (i == 1) {
            rb = obb.halfSize.y;
        } else {
            rb = obb.halfSize.z;
        }

        float proj =
            abs(t[0] * AbsR[0][i] +
                t[1] * AbsR[1][i] +
                t[2] * AbsR[2][i]);

        if (proj > ra + rb) return false;
    }

    return true;
}


bool YoshidaMath::IsCollision(const OBB& a, const OBB& b)
{
    // 数値誤差対策
    const float EPSILON = 1e-5f;

    // 回転行列 R とその絶対値行列 AbsR
    float R[3][3];
    float AbsR[3][3];

    // ベクトル t = b.center - a.center を
    // A のローカル座標系に変換したもの
    Vector3 tWorld{
        b.center.x - a.center.x,
        b.center.y - a.center.y,
        b.center.z - a.center.z
    };

    float t[3];
    t[0] = Function::Dot(tWorld, a.axis[0]);
    t[1] = Function::Dot(tWorld, a.axis[1]);
    t[2] = Function::Dot(tWorld, a.axis[2]);

    // R[i][j] = Ai・Bj
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            R[i][j] = Function::Dot(a.axis[i], b.axis[j]);
        }
    }

    // AbsR[i][j] = |R[i][j]| + EPSILON
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            AbsR[i][j] = fabs(R[i][j]) + EPSILON;
        }
    }

    float ra, rb;

    // =========
    // 1. A の各軸
    // =========
    for (int i = 0; i < 3; ++i) {
        ra = a.halfSize.x * AbsR[i][0] +
            a.halfSize.y * AbsR[i][1] +
            a.halfSize.z * AbsR[i][2];

        rb = b.halfSize.x * AbsR[0][i] +
            b.halfSize.y * AbsR[1][i] +
            b.halfSize.z * AbsR[2][i];

        if (fabs(t[i]) > ra + rb) {
            return false; // 分離軸あり → 衝突していない
        }
    }

    // =========
    // 2. B の各軸
    // =========
    for (int i = 0; i < 3; ++i) {
        ra = a.halfSize.x * AbsR[0][i] +
            a.halfSize.y * AbsR[1][i] +
            a.halfSize.z * AbsR[2][i];

        rb = b.halfSize.x * AbsR[i][0] +
            b.halfSize.y * AbsR[i][1] +
            b.halfSize.z * AbsR[i][2];

        float proj =
            abs(t[0] * R[0][i] +
                t[1] * R[1][i] +
                t[2] * R[2][i]);

        if (proj > ra + rb) {
            return false;
        }
    }

    // =========
    // 3. 交差軸 A0×B0 ～ A2×B2（9本）
    // =========

    // A0 x B0
    ra = a.halfSize.y * AbsR[2][0] + a.halfSize.z * AbsR[1][0];
    rb = b.halfSize.y * AbsR[0][2] + b.halfSize.z * AbsR[0][1];
    if (fabs(t[2] * R[1][0] - t[1] * R[2][0]) > ra + rb) return false;

    // A0 x B1
    ra = a.halfSize.y * AbsR[2][1] + a.halfSize.z * AbsR[1][1];
    rb = b.halfSize.x * AbsR[0][2] + b.halfSize.z * AbsR[0][0];
    if (fabs(t[2] * R[1][1] - t[1] * R[2][1]) > ra + rb) return false;

    // A0 x B2
    ra = a.halfSize.y * AbsR[2][2] + a.halfSize.z * AbsR[1][2];
    rb = b.halfSize.x * AbsR[0][1] + b.halfSize.y * AbsR[0][0];
    if (fabs(t[2] * R[1][2] - t[1] * R[2][2]) > ra + rb) return false;

    // A1 x B0
    ra = a.halfSize.x * AbsR[2][0] + a.halfSize.z * AbsR[0][0];
    rb = b.halfSize.y * AbsR[1][2] + b.halfSize.z * AbsR[1][1];
    if (fabs(t[0] * R[2][0] - t[2] * R[0][0]) > ra + rb) return false;

    // A1 x B1
    ra = a.halfSize.x * AbsR[2][1] + a.halfSize.z * AbsR[0][1];
    rb = b.halfSize.x * AbsR[1][2] + b.halfSize.z * AbsR[1][0];
    if (fabs(t[0] * R[2][1] - t[2] * R[0][1]) > ra + rb) return false;

    // A1 x B2
    ra = a.halfSize.x * AbsR[2][2] + a.halfSize.z * AbsR[0][2];
    rb = b.halfSize.x * AbsR[1][1] + b.halfSize.y * AbsR[1][0];
    if (fabs(t[0] * R[2][2] - t[2] * R[0][2]) > ra + rb) return false;

    // A2 x B0
    ra = a.halfSize.x * AbsR[1][0] + a.halfSize.y * AbsR[0][0];
    rb = b.halfSize.y * AbsR[2][2] + b.halfSize.z * AbsR[2][1];
    if (fabs(t[1] * R[0][0] - t[0] * R[1][0]) > ra + rb) return false;

    // A2 x B1
    ra = a.halfSize.x * AbsR[1][1] + a.halfSize.y * AbsR[0][1];
    rb = b.halfSize.x * AbsR[2][2] + b.halfSize.z * AbsR[2][0];
    if (fabs(t[1] * R[0][1] - t[0] * R[1][1]) > ra + rb) return false;

    // A2 x B2
    ra = a.halfSize.x * AbsR[1][2] + a.halfSize.y * AbsR[0][2];
    rb = b.halfSize.x * AbsR[2][1] + b.halfSize.y * AbsR[2][0];
    if (fabs(t[1] * R[0][2] - t[0] * R[1][2]) > ra + rb) return false;

    // どの軸でも分離できなかった → 衝突している
    return true;
}