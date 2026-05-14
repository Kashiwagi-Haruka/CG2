#include "HintSheet.h"
#include "Model/ModelManager.h"
#include "GameObject/YoshidaMath/YoshidaMath.h"
#include "GameObject/KeyBindConfig.h" // PlayerCommand用
#include "GameObject/SEManager/SEManager.h"
#include "Engine/Loadfile/TXT/TxtManager.h"
#include "application/Color/Color.h"
#include "ScreenSize/ScreenSize.h"
#include"Function.h"
#include"Object3d/Object3dCommon.h"

PlayerCamera* HintSheet::playerCamera_ = nullptr;
namespace {
    const Vector4 kRayHitOutlineColor = { 1.0f, 1.0f, 0.0f, 1.0f };
    const float kRayHitOutlineWidth = 10.0f;
} // namespace
HintSheet::HintSheet()
{
    obj_ = std::make_unique<Object3d>();
    // モデルはDocumentのものを仮置き。適当な板ポリなどに差し替えてOKです。
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/document", "document");
    obj_->SetModel("document");

    // 当たり判定のサイズ
    SetAABB({ .min = {-0.25f, 0.0f, -0.25f}, .max = {0.25f, 0.03f, 0.25f} });
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer);


}

void HintSheet::Initialize(uint32_t fontHandle, const std::string& textFilePath)
{
    isRayHit_ = false;
    isLooking_ = false;
    obj_->Initialize();

    // ーーー Textの初期化 (Credit.cpp準拠) ーーー
    hintText_.Initialize(fontHandle);
    // 画面中央に配置
    hintText_.SetPosition({ SCREEN_SIZE::HALF_WIDTH, SCREEN_SIZE::HALF_HEIGHT });
    hintText_.SetColor(COLOR::WHITE);
    hintText_.SetAlign(TextAlign::Center);


    // 外部ファイルからテキストを読み込む
    try {
        hintText_.SetString(TxtManager::GetInstance()->LoadTxtAsU32String(textFilePath));
    } catch (...) {
        hintText_.SetString(U"NoHint ヒントがあると思った？");
    }

    hintText_.UpdateLayout(false);

    obj_->SetOutlineColor(kRayHitOutlineColor);
    obj_->SetOutlineWidth(kRayHitOutlineWidth);

    //ファイルパスから名前を抽出
    obj_->RegisterEditor(std::filesystem::path(textFilePath).stem().string());
}

void HintSheet::SetParentMatrix(Matrix4x4* parentMatrix)
{
    assert(parentMatrix);
    parentMatrix_ = parentMatrix;

}

void HintSheet::Update()
{
    InteractUpdate();

    obj_->SetEnableLighting(false);
    Transform transform = obj_->GetTransform();

    if (parentMatrix_) {
        //親がセットされていればペアレントする
        Matrix4x4 child = Function::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
        obj_->SetWorldMatrix(Function::Multiply(child, *parentMatrix_));
    } else {
        obj_->SetTransform(transform);
    }

    obj_->Update();

}

void HintSheet::InteractUpdate()
{

    isRayHit_ = playerCamera_->OnCollisionRay(GetAABB(), YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix()));
    // トリガーが押された時の処理
    if (PlayerCommand::GetInstance()->InteractTrigger()) {

        // 物を持っていない状態を前提とする場合
        if (!PlayerCommand::GetIsGrab()) {
            if (isRayHit_) {
                if (isLooking_) {
                    // すでにヒントを見ているなら「閉じる」
                    isLooking_ = false;
                    SEManager::SoundPlay(SEManager::PAPER);
                } else {
                    // ヒントを見ていない ＆ レイが当たっているなら「見る」
                    isLooking_ = true;
                    SEManager::SoundPlay(SEManager::PAPER);
                }
            }
        }
    }

    if (!isRayHit_) {
        //外れたら強制的に終了する
        isLooking_ = false;
    }
}

void HintSheet::Draw()
{


    if (isRayHit_) {
        Object3dCommon::GetInstance()->DrawCommon();
        obj_->Draw();
        Object3dCommon::GetInstance()->DrawCommonOutline();
        obj_->Draw();
        Object3dCommon::GetInstance()->EndOutlineDraw();
    } else {
        Object3dCommon::GetInstance()->DrawCommon();
        obj_->Draw();
    }
}

void HintSheet::DrawUI()
{
    // 見ている状態の時だけテキストを描画する
    if (isLooking_) {
        hintText_.Draw();
    }
}

void HintSheet::SetCamera(Camera* camera)
{
    obj_->SetCamera(camera);
    obj_->UpdateCameraMatrices();
}

void HintSheet::OnCollision(Collider* collider)
{
    // プレイヤーが触れた際の処理が必要なら追加
}

Vector3 HintSheet::GetWorldPosition() const
{
    return YoshidaMath::GetWorldPosByMat(obj_->GetWorldMatrix());
}