#include "Building.h"
#include"Function.h"
#include"imgui.h"
#include"Object3d/Object3dCommon.h"

void BuildingClass::Initialize(const std::string& modelPath, const std::vector<Datas>& datas)
{

    // 1. インスタンス描画オブジェクトの生成と基本初期化
    instancedObject_ = std::make_unique<InstancedObject3d>();
    instancedObject_->Initialize(); // カメラ設定や共通リソースの生成
    // 2. 建物モデルのセット
    instancedObject_->SetModel(modelPath);
    // 3. 座標リストを元に各インスタンスを配置
    SetBuildingPositions(datas);
}

void BuildingClass::Update(const uint32_t floorNum)
{
    if (instancedObject_) {
        instancedObject_->Update();
    }
#ifdef USE_IMGUI
    ImGui::Begin("BuildingPos");

    for (int i = 0; i < datas_.size(); ++i) {
        ImGui::PushID(i);
        ImGui::SliderFloat3("pos", &datas_.at(i).pos.x, -1000.0f, 1000.0f);
        ImGui::SliderFloat("rotY", &datas_.at(i).rotY, -3.1415f, 3.1415f);
        ImGui::PopID();
    }

    ImGui::End();
#endif

    for (int i = 0; i < datas_.size(); ++i) {
        datas_.at(i).pos.y = (floorNum - 1) * -4.0f;
    }

    SetBuildingPositions(datas_);
}

void BuildingClass::Draw()
{
    
    if (instancedObject_) {
        Object3dCommon::GetInstance()->DrawCommon();
        instancedObject_->Draw();
    }
}

void BuildingClass::SetCamera(Camera* camera)
{
    if (instancedObject_) {
        instancedObject_->SetCamera(camera);
        instancedObject_->UpdateCameraMatrices();
    }
}

void BuildingClass::SetBuildingPositions(const std::vector<Datas>& datas)
{
    if (!instancedObject_) return;

    datas_ = datas;

    // 1. インスタンス数を座標の数に合わせてリサイズし、バッファを確保
    instancedObject_->SetInstanceCount(datas_.size());

    // 2. 各建物のインスタンスに任意の座標（オフセット）を適用
    for (size_t i = 0; i < datas_.size(); ++i) {
        instancedObject_->SetInstanceOffset(i, datas_[i].pos);
        instancedObject_->SetInstanceRotate(i, { 0.0f, datas_[i].rotY, 0.0f });
        // instancedObject_->SetInstanceScale(i, {1.0f, 1.0f, 1.0f});
    }
}
