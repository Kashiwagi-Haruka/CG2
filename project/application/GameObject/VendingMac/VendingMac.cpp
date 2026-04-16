#include "VendingMac.h"
#include"Model/ModelManager.h"
#include"Function.h"
#include"GameObject/YoshidaMath/YoshidaMath.h"
#include"GameObject/KeyBindConfig.h"
#include<imgui.h>
#include"GameObject/SEManager/SEManager.h"
#include"Object3d/Object3dCommon.h"
#include <algorithm>
#include <random>

bool VendingMac::isRayHit_ = false;
namespace {
const Vector4 kRayHitOutlineColor = {1.0f, 1.0f, 0.0f, 1.0f};
const float kRayHitOutlineWidth = 26.0f;
} // namespace
VendingMac::VendingMac()
{

    obj_ = std::make_unique<Object3d>();
    drinkEmitter_ = std::make_unique<VendingDrinkEmitter>();
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/vendingMac", "vendingMac");
    obj_->SetModel("vendingMac");
    SetAABB({ .min = {-0.75f,0.0f,-0.75f},.max = {0.75f,1.83f,0.75f} });
    SetCollisionAttribute(kCollisionWall);
    SetCollisionMask(kCollisionPlayer);

    areaLight_.color = { 1.0f,234.0f / 255.0f,200.0f / 255.0f,1.0f };
    areaLight_.intensity = 3.1f;
    areaLight_.width = 2.0f;
    areaLight_.height = 0.8f;
    areaLight_.radius = 0.8f;
    areaLight_.decay = 1.0f;
    translate_ = { 0.0f,1.3f,0.6f };
}

VendingMac::~VendingMac()
{
    SEManager::StopSound(SEManager::NOISE);
}

void VendingMac::OnCollision(Collider* collider)
{
    if (collider->GetCollisionAttribute() == kCollisionPlayer) {

    }
}

Vector3 VendingMac::GetWorldPosition() const
{
    return obj_->GetTranslate();
}

void VendingMac::Update()
{
    CheckCollision();

    obj_->Update();

    Matrix4x4 worldMat = Function::Multiply(Function::MakeTranslateMatrix(translate_), obj_->GetWorldMatrix());
    areaLight_.position = Function::TransformVM({ 0.0f,0.0f,0.0f }, worldMat);
    areaLight_.normal = -1.0f * YoshidaMath::GetForward(obj_->GetWorldMatrix());

    // プレイヤーのカメラ位置から
    Vector3 distance = playerCamera_->GetRay().origin - obj_->GetTranslate();
    float  length = Function::Length(distance);
    SEManager::SetVol(GetVol(length, 1.0f), SEManager::NOISE);

    const Vector3 vendingPosition = obj_->GetTranslate();
    const Vector3 vendingForward = YoshidaMath::GetForward(obj_->GetWorldMatrix());
    drinkEmitter_->SetSpawn({
        vendingPosition.x + vendingForward.x * 0.45f,
        vendingPosition.y + 0.9f,
        vendingPosition.z + vendingForward.z * 0.45f,
    }, vendingForward);
    const float deltaTime = std::max(Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime(), 1.0f / 120.0f);
    drinkEmitter_->Update(deltaTime);


}

void VendingMac::Initialize()
{
    isRayHit_ = false;
	hasPendingResult_ = false;
	pressesWithoutCoffeeMany_ = 0;
    obj_->Initialize();
    drinkEmitter_->Initialize();
    SEManager::SoundPlay(SEManager::NOISE, true);
	obj_->SetOutlineColor(kRayHitOutlineColor);
	obj_->SetOutlineWidth(kRayHitOutlineWidth);
}

void VendingMac::Draw() { 
    if (isRayHit_) {
		Object3dCommon::GetInstance()->DrawCommonOutline();
	} else {
		Object3dCommon::GetInstance()->DrawCommon();
    }
    obj_->Draw(); 
    drinkEmitter_->Draw();
}


void VendingMac::CheckCollision()
{
	static std::random_device rd;
	static std::mt19937 engine(rd());
	static std::uniform_int_distribution<int> distribution(0, 4);

    isRayHit_ = OnCollisionRay();

	if (isRayHit_) {
		if (PlayerCommand::GetInstance()->InteractTrigger()) {
			
                 if (SEManager::IsSoundFinished(SEManager::VENDING_MAC)) {
                SEManager::SoundPlay(SEManager::VENDING_MAC);
            }
            
			DispenseResult result = DispenseResult::Water;
			if (pressesWithoutCoffeeMany_ >= 9) {
				result = DispenseResult::CoffeeMany;
			} else {
				result = static_cast<DispenseResult>(distribution(engine));
			}

			if (result == DispenseResult::CoffeeMany) {
				pressesWithoutCoffeeMany_ = 0;
			} else {
				++pressesWithoutCoffeeMany_;
			}

			pendingResult_ = result;
			hasPendingResult_ = true;
			if (result != DispenseResult::CoffeeMany) {
				drinkEmitter_->SpawnSingle();
			}
		}
	}

}

float VendingMac::GetVol(float length, float maxVol) {
	if (length >= 100.0f) {
		return 0.0f;
	} else if (length > 1.0f) {
		float vol = 1.0f / length;
		return vol * maxVol;
	}

	return maxVol;
}

bool VendingMac::OnCollisionRay() { return playerCamera_->OnCollisionRay(GetAABB(), obj_->GetTranslate()); }

void VendingMac::SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; }

void VendingMac::SetCamera(Camera* camera) {
	obj_->SetCamera(camera);
	obj_->UpdateCameraMatrices();
    drinkEmitter_->SetCamera(camera);
}
Vector3 VendingMac::GetForward() const { return YoshidaMath::GetForward(obj_->GetWorldMatrix()); }

bool VendingMac::ConsumeDispenseResult(DispenseResult& result) {
	if (!hasPendingResult_) {
		return false;
	}

	result = pendingResult_;
	hasPendingResult_ = false;
	return true;
}
