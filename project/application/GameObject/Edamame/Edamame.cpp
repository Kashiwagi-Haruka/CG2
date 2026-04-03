#include "Edamame.h"
#include <Model/ModelManager.h>
#include <GameObject/YoshidaMath/YoshidaMath.h>
#include <GameObject/KeyBindConfig.h>
#include <Function.h>
#include <GameObject/YoshidaMath/CollisionManager/Collider.h>
#include"Object3d/Object3dCommon.h"
#include"GameObject/BGMManager/BGMManager.h"

bool Edamame::isRayHit_ = false;

Edamame::Edamame()
{
    obj_ = std::make_unique<Object3d>();
    // モデルをセット
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/edamame_billboard", "edamame_billboard");
    obj_->SetModel("edamame_billboard");
    //枝豆知識
    edamameTrivia_ = std::make_unique<EdamameTrivia>();
    //枝豆モデル
    edamameModel_ = std::make_unique<EdamameModel>();


}

Edamame::~Edamame()
{

}

void Edamame::Initialize()
{
    isRayHit_ = false;
    worldTransform_ = {
        .scale{1.0f, 1.0f, 1.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{-7.0f, 0.5f, 0.0f}
    };

    obj_->Initialize();
    localAABB_ = { .min = { -0.25f,-0.25f,-0.25f},.max = {0.25f,0.25f,0.25f} };

    //枝豆知識
    edamameTrivia_->Initialize();

    //枝豆モデル
    edamameModel_->Initialize();


    pointLight_.color = { 1.0f, 1.0f, 0.25f, 1.0f };
    pointLight_.position = worldTransform_.translate;
    pointLight_.position.y += 1.0f;
    pointLight_.intensity = 4.0f;
    pointLight_.radius = 4.0f;
    pointLight_.decay = 1.0f;
    pointLight_.shadowEnabled =  true;
}

void Edamame::Update()
{
    obj_->SetEnableLighting(false);
    obj_->SetTransform(worldTransform_);
    obj_->UpdateBillboard();
    Vector3 pos = worldTransform_.translate;
    pos -= playerCamera_->GetRay().diff * 0.25f;
    //スポットライト
    //枝豆モデルに座標をセットする
    edamameModel_->SetTranslate(pos);
    edamameModel_->Update();
    CheckCollision();
    //枝豆知識
    Trivia();
}

void Edamame::Draw()
{
    obj_->Draw();
    edamameModel_->Draw();
}

void Edamame::SetPlayerCamera(PlayerCamera* camera)
{
    playerCamera_ = camera;
}

void Edamame::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
    edamameModel_->SetCamera(camera);
}

void Edamame::CheckCollision()
{
    isRayHit_ = OnCollisionRay();
    //keyとrayの当たり判定
    if (isRayHit_) {
        if (PlayerCommand::GetInstance()->InteractTrigger()) {
            if (!BGMManager::GetIsEdamameSound()) {
                BGMManager::SoundPlay(BGMManager::EDAMAME, false);
                BGMManager::SetIsEdamameSound(true);

                //枝豆モデルのアップデート
                edamameModel_->SetIsStartMove(true);
            }
        }
    }

}

bool Edamame::OnCollisionRay()
{
    return playerCamera_->OnCollisionRay(localAABB_, worldTransform_.translate);
}

void Edamame::Trivia()
{
    //枝豆知識とBGMのvolume変更

    if (BGMManager::GetIsEdamameSound()) {

        if (OnCollisionRay()) {
            edamameTrivia_->Update();
        }


        Vector3 distance = Function::Distance(playerCamera_->GetRay().origin, worldTransform_.translate);
        float  length = Function::Length(distance);
        float bgmVol = 0.0f;
        float vol = 0.0f;

        if (length <= 20.0f) {


            edamameTrivia_->SetIsDraw(true);
            if (length <= 1.0f) {
                bgmVol = 0.25f;
                vol = 1.0f;

            } else {
                vol = 1.0f / length;
                bgmVol = vol * 0.25f;

            }
        } else {
            edamameTrivia_->SetIsDraw(false);
            bgmVol = 0.0f;
            vol = 0.0f;
            //枝豆サウンドを止める
            BGMManager::SetIsEdamameSound(false);
        }

        edamameTrivia_->SetVol(vol);
        BGMManager::SetVol(bgmVol, BGMManager::EDAMAME);
    }


    if (edamameTrivia_->GetIsDie()) {
        //死んだとき 落下開始
       edamameModel_->SetIsDropStart(true);
    }
}
