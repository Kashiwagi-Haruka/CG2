#define NOMINMAX
#include "GiantEnemy.h"
#include"Model/ModelManager.h"
#include"Animation/AnimationManager.h"
#include "GameBase.h"
#include<imgui.h>
#include"Object3d/Object3dCommon.h"
#include<functional>
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
#include"GameObject/SEManager/SEManager.h"
#include"RigidBody.h"
#include"GameObject/YoshidaMath/Easing.h"

namespace {
    AABB localAABB = { .min = {-1.0f,-1.0f,-1.0f},.max = {1.0f,1.0,1.0f} };
    static constexpr float kWalkFootstepInterval = 1.0f;
    static constexpr float kWalkMoveLen_ = 20.0f;
    static constexpr float kSneakMoveLen_ = 10.0f;
    static constexpr float kWalkSpeed_ = 2.0f;
    static constexpr float kSneakSpeed_ = 1.0f;
    static constexpr float kDuretion_ = 0.5f;
}
Vector3* GiantEnemy::targetPos_ = nullptr;

GiantEnemy::GiantEnemy()
{
    obj_ = std::make_unique<Object3d>();
    obj_->SetModel("gentleman");
}

void GiantEnemy::Initialize()
{

    desiredAnimationName = "Idle";

    soundTimer_ = 0.0f;

    obj_->Initialize();
    obj_->RegisterEditor(animationGroupName_);

    collisionTransform_.scale = { -1.5f / 1.6f,-1.5f / 0.9f,-1.5f };
    collisionTransform_.rotate = { 0.0f,0.0f,0.0f };
    SetCollisionAttribute(kCollisionEnemy);

    velocity_ = { 0.0f };

    AnimationManager::GetInstance()->LoadAnimationGroup(animationGroupName_, "Resources/TD3_3102/3d/gentleman", "gentleman");
    AnimationManager::GetInstance()->ResetPlayback(animationGroupName_, desiredAnimationName, false);
    if (const Animation::AnimationData* idleAnimation = AnimationManager::GetInstance()->FindAnimation(animationGroupName_, desiredAnimationName)) {
        obj_->SetAnimation(idleAnimation, false);
    }

    if (Model* gentlemanModel = ModelManager::GetInstance()->FindModel("gentleman")) {
        skeleton_ = std::make_unique<Skeleton>(Skeleton().Create(gentlemanModel->GetModelData().rootnode));
        skinCluster_ = CreateSkinCluster(*skeleton_, *gentlemanModel);
        if (!skinCluster_.mappedPalette.empty()) {
            obj_->SetSkinCluster(&skinCluster_);
        }
    }

    std::string leftFoot = animationGroupName_ + "Foot_L";
    parentMats_[leftFoot] = Function::MakeIdentity4x4();
    colliders_[leftFoot] = std::make_unique<ObjectCollider>();
    colliders_[leftFoot]->SetParentMatrix(&parentMats_[leftFoot]);

    std::string rightFoot = animationGroupName_ + "Foot_R";
    parentMats_[rightFoot] = Function::MakeIdentity4x4();
    colliders_[rightFoot] = std::make_unique<ObjectCollider>();
    colliders_[rightFoot]->SetParentMatrix(&parentMats_[rightFoot]);

    for (auto& [name, collider] : colliders_) {
        collider->Initialize(YoshidaMath::ColliderType::kSphere);
        collider->SetCollisionAttribute(kCollisionEnemy);
        collider->SetCollisionMask(kCollisionPlayer | kCollisionFloor);
        collider->SetParentScale(&obj_->GetScalePtr());
    }

}

void GiantEnemy::SetName(const std::string name)
{
    animationGroupName_ = name;
}

void GiantEnemy::Update()
{

    ResetFootContactState();

    if (!IsWall()) {
        const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
        soundTimer_ = std::max(0.0f, soundTimer_ - deltaTime);
    }

    Transform transfrom = obj_->GetTransform();

    if (!IsWall()) {
        if (targetPos_) {

            Vector3 distance = Function::Distance(*targetPos_, transfrom.translate);
            //距離と連動して動くよ
            float length = Function::Length(distance);

            if (length < kWalkMoveLen_) {
                YoshidaMath::LookTarget(*targetPos_, transfrom);
                velocity_ = YoshidaMath::GetToTargetVec(*targetPos_, transfrom.translate);
                const float deltaTime = GameBase::GetInstance()->GetDeltaTime();

                desiredAnimationName = "Sneak";

                if (length < kSneakMoveLen_) {      
                    velocity_ *= deltaTime * kSneakSpeed_;
                } else {
                    velocity_ *= deltaTime * kWalkSpeed_;
                }
                transfrom.translate.x += velocity_.x;
                transfrom.translate.z += velocity_.z;
            } else {
                desiredAnimationName = "Idle";
            }

        }
    }

    transfrom.rotate = YoshidaMath::Easing::Lerp( obj_->GetTransform().rotate, transfrom.rotate, kDuretion_);
    transfrom.translate = YoshidaMath::Easing::Lerp(obj_->GetTransform().translate, transfrom.translate, kDuretion_);

    obj_->SetTransform(transfrom);
    obj_->Update();
    skeleton_->SetObjectMatrix(obj_->GetWorldMatrix());

    Animation();

    //舌のコントロールボーン
    collisionTransform_.translate = GetJointWordPos("CON.tongue.002");

    std::string leftFoot = animationGroupName_ + "Foot_L";
    std::string rightFoot = animationGroupName_ + "Foot_R";
    parentMats_[leftFoot] = GetJointWorldMatrix("toe.L");
    parentMats_[rightFoot] = GetJointWorldMatrix("toe.R");

    for (auto& [name, collider] : colliders_) {
        Transform transform = { .scale = {0.375f,0.375f,0.375f} ,  .rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,-0.1f,0.0f} };
        collider->SetTransform(transform);
        collider->Update();
    }

    //壁になったらセピアになるよ
    obj_->SetSepiaEnabled(IsWall());

#ifdef USE_IMGUI

    ImGui::Begin("GiantGentleMan");

    if (ImGui::TreeNode(animationGroupName_.c_str())) {
        ImGui::Text("CurrantAnimation : %s", desiredAnimationName.c_str());
        ImGui::TreePop();
    }

    ImGui::End();

#endif
}

void GiantEnemy::Draw()
{
    Object3dCommon::GetInstance()->DrawCommonSkinning();
    obj_->Draw();
    //Object3dCommon::GetInstance()->DrawCommon();
    //for (auto& [name, collider] : colliders_) {
    //    collider->Draw();
    //}
}

void GiantEnemy::ResetCollisionAttribute()
{
    SetCollisionAttribute(kCollisionEnemy);
    for (auto& [name, collider] : colliders_) {
        collider->SetCollisionAttribute(kCollisionEnemy);
    }
}

void GiantEnemy::SetCollisionAttributeWallAndInitialize()
{
    SetCollisionAttribute(kCollisionWall);
    for (auto& [name, collider] : colliders_) {
        collider->SetCollisionAttribute(kCollisionWall);
    }
    //壁になったら あのぽーすをしてくれます。
    desiredAnimationName = "AerialPigeon";

}
void GiantEnemy::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();

    //for (auto& [name, collider] : colliders_) {
    //    collider->SetCamera(camera);
    //}
}

bool GiantEnemy::IsFacingSurface(const Matrix4x4& cameraMat)
{
    Vector3 forward = YoshidaMath::GetForward(cameraMat);

    const std::optional<int32_t> jointIndex = skeleton_->FindJointIndex("CON.tongue.002");
    Matrix4x4 mat = skeleton_->GetJointWorldMatrix(skeleton_->GetJoints()[*jointIndex]);
    Vector3 direction = YoshidaMath::GetForward(mat);
    float dot = Function::Dot(forward, direction);

    return(dot >= -kPortalCreatableAngleRange_);
}

void GiantEnemy::OnCollisionWithFloor(YoshidaMath::Collider* collider)
{

    if (IsWall()|| desiredAnimationName == "Idle") {
        return;
    }

    if (!collider) {
        return;
    }

    const uint32_t collisionAttribute = collider->GetCollisionAttribute();

    if (collisionAttribute != kCollisionFloor) {
        return;
    }

    std::string leftFoot = animationGroupName_ + "Foot_L";
    std::string rightFoot = animationGroupName_ + "Foot_R";

   const Sphere footL = YoshidaMath::GetSphereWorldPos(colliders_[leftFoot].get());
   const Sphere footR = YoshidaMath::GetSphereWorldPos(colliders_[rightFoot].get());
   const AABB floor = YoshidaMath::GetAABBWorldPos(collider);

 
   leftFootGrounded_=RigidBody::isCollision(floor, footL);
    rightFootGrounded_ =  RigidBody::isCollision(floor, footR);

    if (leftFootGrounded_ ||  rightFootGrounded_) {

        if (soundTimer_ > 0.0f) {
            return;
        }

        SEManager::SoundPlay(SEManager::FOOT_STEP_BIG);
        soundTimer_ = kWalkFootstepInterval;
    }
}


bool GiantEnemy::OnCollisionRay(PlayerCamera* playerCamera)
{
    //かなり遠くからでも打てるようにする
    return playerCamera->OnCollisionRay(localAABB, collisionTransform_.translate, 0.0f, 2000.0f);
}

void GiantEnemy::Animation()
{
    bool loopAnimation = true;

    if (IsWall()) {
        loopAnimation = false;
    }

    const float deltaTime = GameBase::GetInstance()->GetDeltaTime();
    AnimationManager::PlaybackResult playbackResult{};

    if (AnimationManager::GetInstance()->UpdatePlayback(animationGroupName_, desiredAnimationName, loopAnimation, deltaTime, kAnimationBlendDuration_, blendedPoseAnimation_, playbackResult)) {
        animationFinished_ = playbackResult.animationFinished;

        if (playbackResult.changedAnimation && playbackResult.currentAnimation) {
            obj_->SetAnimation(playbackResult.currentAnimation, loopAnimation);
        }

        if (skeleton_ && playbackResult.animationToApply) {
            skeleton_->ApplyAnimation(*playbackResult.animationToApply, playbackResult.animationTime);
            skeleton_->Update();
            if (!skinCluster_.mappedPalette.empty()) {
                UpdateSkinCluster(skinCluster_, *skeleton_);
            }
        }
    }
}

const Vector3 GiantEnemy::GetJointWordPos(const std::string name)
{
    const std::optional<int32_t> jointIndex = skeleton_->FindJointIndex(name);
    return skeleton_->GetJointWorldPosition(skeleton_->GetJoints()[*jointIndex]);

}

const Matrix4x4 GiantEnemy::GetJointWorldMatrix(const std::string name)
{
    const std::optional<int32_t> jointIndex = skeleton_->FindJointIndex(name);
    return skeleton_->GetJointWorldMatrix(skeleton_->GetJoints()[*jointIndex]);
}

void GiantEnemy::ResetFootContactState()
{
    leftFootGrounded_ = false;
    rightFootGrounded_ = false;
}
