#include "Edamame.h"
#include <Model/ModelManager.h>
#include <GameObject/YoshidaMath/YoshidaMath.h>
#include <GameObject/KeyBindConfig.h>
#include <Function.h>
#include <GameObject/YoshidaMath/CollisionManager/Collider.h>
#include"Object3d/Object3dCommon.h"
#include"GameObject/BGMManager/BGMManager.h"
Edamame::Edamame()
{
    obj_ = std::make_unique<Object3d>();
    // モデルをセット
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/edamame_billboard", "edamame_billboard");
    obj_->SetModel("edamame_billboard");
    //枝豆知識
    edamameTrivia_ = std::make_unique<EdamameTrivia>();
}

Edamame::~Edamame()
{

}

void Edamame::Initialize()
{
    worldTransform_ = {
        .scale{0.5f, 0.5f, 0.5f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{-4.0f, 0.25f, 0.0f}
    };

    obj_->Initialize();
    localAABB_ = { .min = { -0.5f,-0.5f,-0.5f},.max = {0.5f,0.5f,0.5f} };

    //枝豆知識
    edamameTrivia_->Initialize();

}

void Edamame::Update()
{
    obj_->SetTransform(worldTransform_);
    obj_->UpdateBillboard();

}

void Edamame::Draw()
{
    obj_->Draw();
}

void Edamame::SetPlayerCamera(PlayerCamera* camera)
{
    playerCamera_ = camera;
}

void Edamame::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}

void Edamame::SetModel(const std::string& filePath)
{
    obj_->SetModel(filePath);
}

void Edamame::CheckCollision()
{
    //keyとrayの当たり判定
    if (OnCollisionRay()) {
        if (PlayerCommand::GetInstance()->Interact()) {
            // カーソルに追従させて持ち上げる処理
            Vector3 origin = playerCamera_->GetTransform().translate;
            worldTransform_.translate = origin + (Function::Normalize(playerCamera_->GetRay().diff));
            worldTransform_.translate.y = (std::max)(worldTransform_.translate.y, 0.25f);
        }

        if (PlayerCommand::GetInstance()->InteractTrigger()) {
            if (!BGMManager::GetIsEdamameSound()) {
                BGMManager::SoundPlay(BGMManager::EDAMAME, false);
                BGMManager::SetIsEdamameSound(true);
            }
        }     
    }

    //枝豆知識
    Trivia();
}

bool Edamame::OnCollisionRay()
{
    return playerCamera_->OnCollisionRay(localAABB_, worldTransform_.translate);
}

void Edamame::Trivia()
{
    //枝豆知識とBGMのvolume変更

    if (BGMManager::GetIsEdamameSound()) {

        edamameTrivia_->Update();

        Vector3 distance = Function::Distance(playerCamera_->GetRay().origin, worldTransform_.translate);
        float  length = Function::Length(distance);
        float bgmVol = 0.0f;
        float vol = 0.0f;

        if (length <= 50.0f) {
            if (length <= 1.0f) {
                bgmVol = 0.25f;
                vol = 1.0f;
            } else {
                vol = 1.0f / length;
                bgmVol = vol * 0.25f;
            }
        } else {
            bgmVol = 0.0f;
            vol = 0.0f;
            //枝豆サウンドを止める
        /*    BGMManager::SetIsEdamameSound(false);*/
        }

        edamameTrivia_->SetVol(vol);
        BGMManager::SetVol(bgmVol, BGMManager::EDAMAME);
    }
}
