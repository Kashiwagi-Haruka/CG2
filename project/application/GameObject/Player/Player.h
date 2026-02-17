#pragma once
#include<memory>
#include"Object3d/Object3d.h"
#include "Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"
class Camera;

class Player
{
private:
#pragma region//体やメッシュの情報
    //体のObj
    std::unique_ptr<Object3d> bodyObj_ = nullptr;

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
public:
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
};

