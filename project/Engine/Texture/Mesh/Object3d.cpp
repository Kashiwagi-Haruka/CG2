#include "Object3d.h"
#include "Object3dCommon.h"
#include "DirectXCommon.h"
#include <cassert>
#include "Function.h"
#include "TextureManager.h"
#include "ModelManeger.h"
#include "Model.h"
#include "Camera.h"

void Object3d::Initialize(Object3dCommon* modelCommon){ 
	obj3dCommon_ = modelCommon;
	camera_ = obj3dCommon_->GetDefaultCamera();
	CreateResources();
	
}
void Object3d::Update(){
	// [0]=モデル描画用で使う
	
	Matrix4x4 worldMatrix = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 worldViewProjectionMatrix;

	if (camera_) {
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Function::Multiply(worldMatrix, viewProjectionMatrix);
	} else {
		worldViewProjectionMatrix = worldMatrix;
	}


	transformResource_ = obj3dCommon_->CreateBufferResource(sizeof(TransformationMatrix));
	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix;

}
void Object3d::Draw() {

	// --- 座標変換行列CBufferの場所を設定 ---
	obj3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress());
	// --- 平行光源CBufferの場所を設定 ---
	obj3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

	if (model_) {
		model_->Draw();
	}
	
}

void Object3d::SetModel(const std::string& filePath) { model_ = ModelManeger::GetInstance()->FindModel(filePath); }
void Object3d::SetCamera(Camera* camera) { camera_ = camera; }
void Object3d::SetScale(Vector3 scale){ transform_.scale = scale; }
void Object3d::SetRotate(Vector3 rotate) { transform_.rotate = rotate; }
void Object3d::SetTranslate(Vector3 translate) { transform_.translate = translate; }

void Object3d::CreateResources() {
	

	Update();

	// Lightバッファ
	directionalLightResource_ = obj3dCommon_->CreateBufferResource(sizeof(DirectionalLight));
	assert(directionalLightResource_);
	directionalLightData_ = nullptr;
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	assert(directionalLightData_);
	*directionalLightData_ = {
	    {1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, -1.0f, 0.0f},
        1.0f
    };
}