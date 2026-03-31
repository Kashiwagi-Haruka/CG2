#include "CollisionManager.h"
#include "Collider.h"
#include"RigidBody.h"
#include"Object3d/Object3dCommon.h"
using namespace YoshidaMath;

void CollisionManager::CheckAllCollisions() {
    // リスト内のペアを総当たり
    std::list<YoshidaMath::Collider*>::iterator itrA = colliders_.begin();
    for (; itrA != colliders_.end(); ++itrA) {

        std::list<YoshidaMath::Collider*>::iterator itrB = itrA;
        for (++itrB; itrB != colliders_.end(); ++itrB) {
            // 衝突フィルタリング
            if (((*itrA)->GetCollisionAttribute() & (*itrB)->GetCollisionMask()) == 0 ||
                ((*itrB)->GetCollisionAttribute() & (*itrA)->GetCollisionMask()) == 0) {
                continue; // 衝突しない
            }

            CheckCollisionPair(*itrA, *itrB);

        }
    }
}

void CollisionManager::CheckCollisionSpherePair(YoshidaMath::Collider* colliderA, YoshidaMath:: Collider* colliderB)
{
    // 衝突判定
    if (YoshidaMath::IsCollision(YoshidaMath::GetSphereWorldPos(colliderA), YoshidaMath::GetSphereWorldPos(colliderB))) {
        OnCollision(colliderA, colliderB);
    }
}

void CollisionManager::CheckCollisionAABBPair(YoshidaMath::Collider* colliderA, YoshidaMath:: Collider* colliderB)
{
    AABB worldPosA = GetAABBWorldPos(colliderA);
    AABB worldPosB = GetAABBWorldPos(colliderB);

    colliderA->SetCollisionInfo(GetCollisionInfo(worldPosA, worldPosB));
    colliderB->SetCollisionInfo(GetCollisionInfo(worldPosA, worldPosB));

    // 衝突判定
    if (colliderA->GetCollisionInfo().collided&& colliderB->GetCollisionInfo().collided) {
        OnCollision(colliderA, colliderB);
    }
}

void CollisionManager::CheckCollisionSphereAABBPair(YoshidaMath::Collider* sphereC, YoshidaMath:: Collider* aabbC)
{
    // 衝突判定
    if (RigidBody::isCollision(GetAABBWorldPos(aabbC), GetSphereWorldPos(sphereC))) {
        OnCollision(aabbC, sphereC);
    }
}

void CollisionManager::CheckCollisionOBBPair(YoshidaMath::Collider* colliderA, YoshidaMath::Collider* colliderB)
{
    YoshidaMath::UpdateOBB(colliderA);
    YoshidaMath::UpdateOBB(colliderB);
    if (YoshidaMath::IsCollision(colliderA->GetOBB(), colliderB->GetOBB())) {
        OnCollision(colliderA, colliderB);
    }
}

void CollisionManager::CheckCollisionAABBOBBPair(YoshidaMath::Collider* colliderA, YoshidaMath::Collider* colliderB)
{
    YoshidaMath::UpdateOBB(colliderB);

    if (YoshidaMath::IsCollision(GetAABBWorldPos(colliderA), colliderB->GetOBB())) {
        OnCollision(colliderA, colliderB);
    }
}

void CollisionManager::CheckCollisionPair(YoshidaMath::Collider* a, YoshidaMath::Collider* b) {
    auto typeA = a->GetType();
    auto typeB = b->GetType();

    if (typeA == YoshidaMath::kSphere && typeB == YoshidaMath::kSphere) {
        CheckCollisionSpherePair(a, b);
    } else if (typeA == YoshidaMath::kSphere && typeB == YoshidaMath::kAABB) {
        CheckCollisionSphereAABBPair(a, b);
    } else if (typeA == YoshidaMath::kAABB && typeB == YoshidaMath::kSphere) {
        CheckCollisionSphereAABBPair(b, a); // 順番に注意！
    } else if (typeA == YoshidaMath::kAABB && typeB == YoshidaMath::kAABB) {
        CheckCollisionAABBPair(a, b);
    } else if (typeA == YoshidaMath::kOBB, typeB == YoshidaMath::kOBB) {
        CheckCollisionOBBPair(a, b);
    } else if (typeA == YoshidaMath::kAABB, typeB == YoshidaMath::kOBB) {
        CheckCollisionAABBOBBPair(a, b);
    } else if (typeA == YoshidaMath::kOBB, typeB == YoshidaMath::kAABB) {
        CheckCollisionAABBOBBPair(b, a);
    }
}

void CollisionManager::OnCollision(YoshidaMath::Collider* a, YoshidaMath::Collider* b)
{
    a->OnCollision(b);
    b->OnCollision(a);
}
