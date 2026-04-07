#include "IDCard.h"
#include "TextureManager.h"
#include"ScreenSize/ScreenSize.h"
#include"SpriteCommon.h"
#include"GameBase.h"
#include"GameObject/YoshidaMath/Easing.h"
#include<algorithm>
#include"Function.h"

IDCard::IDCard()
{
    handle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/IDcard.png");
    sprite_ = std::make_unique<Sprite>();
}

void IDCard::Initialize()
{
    rotate_ = 0.0f;
    rotateCoolTimer_ = 0.0f;
    startRotate_ = 0.0f;
    endRotate_ = 0.0f;
    rotateCount_ = 0;
    isRotateStart = false;
    isEaseEnd_ = false;
    sprite_->Initialize(handle_);
    sprite_->SetAnchorPoint({ 0.5f,0.0f });
    sprite_->SetRotation(0.0f);
    sprite_->SetScale({ 256,230.0f });
    sprite_->SetPosition({ SCREEN_SIZE::WIDTH - 64.0f - 128.0f,-16.0f});
    sprite_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
    sprite_->Update();
}

void IDCard::Update()
{
    const float deltaTime = GameBase::GetInstance()->GetDeltaTime();
    rotateCoolTimer_ -= deltaTime;

    if (rotateCoolTimer_ <= 0.0f) {
        rotateCoolTimer_ = kRotateCoolTime_;
        isRotateStart = true;
    }

    if (isRotateStart) {

        if (isEaseEnd_) {

            float theta = Function::kPi * 0.25f;
            
            rotateCount_++;

            if (rotateCount_ == 1) {
                SetRotate(0.0f, theta,0.5f);
            } else if (rotateCount_ == 2) {
                SetRotate(theta, -theta*0.25f,1.0f);
            } else if (rotateCount_ == 3) {
                SetRotate(-theta * 0.25f, 0.0f,0.5f);
            } else {
                rotateCount_ = 0;
                isRotateStart = false;
                rotateCount_ = 0;
            }

   
        }

        rotateTimer_ += deltaTime/lerpTime_;
        rotateTimer_ = std::clamp(rotateTimer_, 0.0f, 1.0f);
        isEaseEnd_ = (rotateTimer_ == 1.0f);
        rotate_ = YoshidaMath::Easing::EaseInOutCubic(startRotate_, endRotate_, rotateTimer_);
        sprite_->SetRotation(rotate_);
    }


    sprite_->Update();
}

void IDCard::Draw()
{
    SpriteCommon::GetInstance()->SetBlendMode(BlendMode::kBlendModeAlpha);
    SpriteCommon::GetInstance()->DrawCommon();
    sprite_->Draw();
}

void IDCard::InitCount()
{
    rotate_ = 0.0f;
    rotateCoolTimer_ = 0.0f;
    rotateCount_ = 0;
    isRotateStart = false;
}

void IDCard::SetRotate(const float start, const float end,const float lerpTime)
{
    startRotate_ = start;
    endRotate_ = end;
    lerpTime_ = lerpTime;
    rotateTimer_ = 0.0f;
    isEaseEnd_ = false;
}
