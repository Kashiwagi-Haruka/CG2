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
	
	worldMatrix = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	worldViewProjectionMatrix;

	if (camera_) {
		viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Function::Multiply(worldMatrix, viewProjectionMatrix);
	} else {
		worldViewProjectionMatrix = worldMatrix;
	}



	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix;
	transformResource_->Unmap(0, nullptr);
	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));
	if (camera_) {
		*cameraData_ = camera_->GetWorldTranslate();
	} else {
		*cameraData_ = {worldMatrix.m[3][0], worldMatrix.m[3][1], worldMatrix.m[3][2]};
	}
	
	
	cameraResource_->Unmap(0, nullptr);

}
void Object3d::Draw() {



	// --- 座標変換行列CBufferの場所を設定 ---
	obj3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress());
	// --- 平行光源CBufferの場所を設定 ---
	obj3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, obj3dCommon_->GetDirectionalLightResource()->GetGPUVirtualAddress());
	obj3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource_->GetGPUVirtualAddress());

	if (model_) {
		model_->Draw();
	}
	
}

void Object3d::SetModel(const std::string& filePath) { model_ = ModelManeger::GetInstance()->FindModel(filePath); }
void Object3d::SetCamera(Camera* camera) { camera_ = camera; }
void Object3d::SetScale(Vector3 scale){ transform_.scale = scale; }
void Object3d::SetRotate(Vector3 rotate) { transform_.rotate = rotate; }
void Object3d::SetTranslate(Vector3 translate) { transform_.translate = translate; }
void Object3d::SetColor(Vector4 color) {
	if (model_) {
		model_->SetColor(color);
	}
}
void Object3d::SetEnableLighting(bool enable) {
	if (model_) {
		model_->SetEnableLighting(enable);
	}
}
void Object3d::SetUvTransform(const Matrix4x4& uvTransform) {
	if (model_) {
		model_->SetUvTransform(uvTransform);
	}
}
void Object3d::SetShininess(float shininess) {
	if (model_) {
		model_->SetShininess(shininess);
	}
}
void Object3d::CreateResources() {
	transformResource_ = obj3dCommon_->CreateBufferResource(sizeof(TransformationMatrix));
	cameraResource_ = obj3dCommon_->CreateBufferResource(sizeof(Vector3));
	


}