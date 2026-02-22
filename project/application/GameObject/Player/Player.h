#pragma once
#include<memory>
#include"Object3d/Object3d.h"
#include "Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"
#include"RigidBody.h"
#ifdef _DEBUG
#include"Primitive/Primitive.h"
#endif
class Camera;

class Player
{
private:
#pragma region//体やメッシュの情報
    //体のObj
    std::unique_ptr<Object3d> bodyObj_ = nullptr;
#ifdef _DEBUG
    std::unique_ptr<Primitive> primitive_ = nullptr;
#endif
    //アニメーションクリップ
    std::vector<Animation::AnimationData> animationClips_{};
    //現在のアニメーションインデックス
    size_t currentAnimationIndex_ = 0;
    //骨
    std::unique_ptr<Skeleton> skeleton_{};
    //スキン
    SkinCluster skinCluster_{};
    //アニメーション時間
    float animationTime_ = 0.0f;
#pragma endregion
    //体のtransform
    Transform transform_{};
    Vector3 velocity_ = { 0.0f };
    float speed_ = { 0.0f };
    Vector3 tempDirection_ = { 0.0f };

    //カメラの感度をここで宣言していて良くない
    float eyeRotateSpeed_ = 0.3f;
    float eyeRotateX_ = 0.0f;
    AABB localAABB_ = { 0.0f };
public:
    Transform& GetTransform() { return transform_; };
    //コンストラクタ
    Player();
    //カメラのセッター
    void SetCamera(Camera* camera);
    //初期化
    void Initialize();
    //更新処理
    void Update();
    //描画処理
    void Draw();
    //デバック
    void Debug();
    //移動
    void Move();
    //回転
    void Rotate();
    //アニメーション
    void Animation();
    //ワールド行列の取得
    const Matrix4x4& GetWorldMatrix()const { return bodyObj_->GetWorldMatrix(); }
    //ワールド座標のAABBの取得
    AABB GetWorldAABB();
};

