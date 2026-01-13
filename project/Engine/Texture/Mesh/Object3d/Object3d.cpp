#include "Object3d/Object3d.h"
#include "Object3d/Object3dCommon.h"
#include "DirectXCommon.h"
#include <cassert>
#include "Function.h"
#include "TextureManager.h"
#include "Model/ModelManeger.h"
#include "Model/Model.h"
#include "Camera.h"


void Object3d::Initialize(){ 
	
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();
	CreateResources();
	isUseSetWorld = false;
}
void Object3d::Update(){
	// [0]=モデル描画用で使う
	
	if (!isUseSetWorld) {
	worldMatrix = Function::Multiply(model_->GetModelData().rootnode.localMatrix ,Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate));
	} else {
		worldMatrix = Function::Multiply(model_->GetModelData().rootnode.localMatrix, worldMatrix);
	}
	
	
	worldViewProjectionMatrix = Function::Multiply(model_->GetModelData().rootnode.localMatrix , Function::Multiply(Function::Multiply(worldMatrix,camera_->GetViewMatrix()),camera_->GetProjectionMatrix()));
	
		
	

	

	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));

	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix;

	transformationMatrixData_->WorldInverseTranspose = Function::Inverse(worldMatrix);
	transformResource_->Unmap(0, nullptr);
	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));
	if (camera_) {
		cameraData_->worldPosition = camera_->GetWorldTranslate();

	} else {
		cameraData_->worldPosition = {transform_.translate};
	}
	cameraResource_->Unmap(0, nullptr);
	
}
void Object3d::Draw() {



	// --- 座標変換行列CBufferの場所を設定 ---
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress());
	// --- 平行光源CBufferの場所を設定 ---
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, Object3dCommon::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource_->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(5, Object3dCommon::GetInstance()->GetPointLightCountResource()->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(6, Object3dCommon::GetInstance()->GetSpotLightResource()->GetGPUVirtualAddress());
	if (model_) {
		model_->Draw();
	}
	
}

void Object3d::SetModel(const std::string& filePath) { model_ = ModelManager::GetInstance()->FindModel(filePath); }
void Object3d::SetCamera(Camera* camera) { camera_ = camera; }
void Object3d::SetScale(Vector3 scale) {
	transform_.scale = scale;
	isUseSetWorld = false;
}
void Object3d::SetRotate(Vector3 rotate) {
	transform_.rotate = rotate;
	isUseSetWorld = false;
}
void Object3d::SetTranslate(Vector3 translate) {
	transform_.translate = translate;
	isUseSetWorld = false;
}
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
	transformResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
	cameraResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(CameraForGpu));
	


}