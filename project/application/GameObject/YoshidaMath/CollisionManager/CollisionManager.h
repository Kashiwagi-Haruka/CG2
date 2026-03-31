#pragma once

#include <list>

#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
class Camera;

/// @brief 衝突マネージャ
class CollisionManager {
public:

    /// @brief コライダーを追加する
    /// @param collider コライダー
    void AddCollider(YoshidaMath::Collider* collider) {
        colliders_.emplace_back(collider); }

    template <typename Container>
    void AddColliders(const Container& colliders)
    {
        for (auto& collider : colliders) {
            colliders_.emplace_back(collider);
        }
    }
    /// @brief コライダーをクリアする
    void ClearColliders() { colliders_.clear(); }
    /// @brief 衝突判定と応答
    void CheckAllCollisions();
private:
    std::list<YoshidaMath::Collider*> colliders_; // コライダーのリスト

    void  CheckCollisionPair(YoshidaMath::Collider* a, YoshidaMath::Collider* b);
    void OnCollision(YoshidaMath::Collider* a, YoshidaMath::Collider* b);
    /// @brief コライダー2つの衝突判定と応答
    /// @param colliderA コライダーA
    /// @param colliderB コライダーB
    /// @param score スコアポインタ
    void CheckCollisionSpherePair(YoshidaMath::Collider* colliderA, YoshidaMath::Collider* colliderB);

    /// @brief コライダー2つの衝突判定と応答
/// @param colliderA コライダーA
/// @param colliderB コライダーB
/// @param score スコアポインタ
    void CheckCollisionAABBPair(YoshidaMath::Collider* colliderA, YoshidaMath::Collider* colliderB);

    /// @brief コライダー2つの衝突判定と応答
/// @param colliderA コライダーA
/// @param colliderB コライダーB
/// @param score スコアポインタ
    void CheckCollisionSphereAABBPair(YoshidaMath::Collider* colliderA, YoshidaMath::Collider* colliderB);
    /// @brief コライダー2つの衝突判定と応答
/// @param colliderA コライダーA
/// @param colliderB コライダーB
/// @param score スコアポインタ
    void CheckCollisionOBBPair(YoshidaMath::Collider* colliderA, YoshidaMath::Collider* colliderB);
    /// @param colliderA コライダーA
/// @param colliderB コライダーB
/// @param score スコアポインタ
    void CheckCollisionAABBOBBPair(YoshidaMath::Collider* colliderA, YoshidaMath::Collider* colliderB);

};