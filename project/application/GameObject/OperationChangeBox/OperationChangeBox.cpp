#include "OperationChangeBox.h"
#include "Engine/math/Function.h"
#include "GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include "GameObject/KeyBindConfig.h"
#include "Primitive/Primitive.h"

namespace {
constexpr Vector4 kRayHitColor = {1.0f, 0.95f, 0.5f, 1.0f};
constexpr Vector4 kDefaultColor = {0.75f, 0.9f, 1.0f, 1.0f};
}

void OperationChangeBox::Initialize() {
    box_ = std::make_unique<Primitive>();
    box_->Initialize(Primitive::Box, "Resources/TD3_3102/2d/white2x2.png");
    transform_ = {
        .scale = {0.9f, 0.75f, 0.9f},
        .rotate = {0.0f, Function::kPi, 0.0f},
        .translate = {0.0f, 0.75f, -3.0f},
    };
    box_->SetTransform(transform_);
    box_->SetEnableLighting(true);
    box_->SetColor(kDefaultColor);
}

void OperationChangeBox::Update() {
    if (!box_) {
        return;
    }

    isRayHit_ = OnCollisionRay();
    if (isRayHit_ && PlayerCommand::GetInstance()->InteractTrigger()) {
        interactRequested_ = true;
    }

    box_->SetTransform(transform_);
    box_->SetColor(isRayHit_ ? kRayHitColor : kDefaultColor);
    box_->Update();
}

void OperationChangeBox::Draw() {
    if (box_) {
        box_->Draw();
    }
}

void OperationChangeBox::SetCamera(Camera* camera) {
    if (!box_) {
        return;
    }

    box_->SetCamera(camera);
    box_->UpdateCameraMatrices();
}

bool OperationChangeBox::ConsumeInteractRequest() {
    const bool requested = interactRequested_;
    interactRequested_ = false;
    return requested;
}

Vector3 OperationChangeBox::GetForward() const { return Function::MakeForwardFromRotate(transform_.rotate); }

AABB OperationChangeBox::GetAABB() const {
    return {
        .min = {-transform_.scale.x * 0.5f, -transform_.scale.y * 0.5f, -transform_.scale.z * 0.5f},
        .max = {transform_.scale.x * 0.5f, transform_.scale.y * 0.5f, transform_.scale.z * 0.5f},
    };
}

bool OperationChangeBox::OnCollisionRay() const {
    if (!playerCamera_) {
        return false;
    }
    return playerCamera_->OnCollisionRay(GetAABB(), transform_.translate);
}
