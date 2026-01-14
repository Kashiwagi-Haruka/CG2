#include "Object3d/Object3d.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "Function.h"
#include "Model/Model.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include "TextureManager.h"
#include <algorithm>
#include <cassert>
#include <cmath>

void Object3d::Initialize() {

	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();
	CreateResources();
	isUseSetWorld = false;
}
void Object3d::Update() {
	// [0]=モデル描画用で使う

	Matrix4x4 localMatrix = model_ ? model_->GetModelData().rootnode.localMatrix : Function::MakeIdentity4x4();
	if (animation_ && model_) {
		animationTime_ += 1.0f / 60.0f;
		if (animation_->duration > 0.0f) {
			if (isLoopAnimation_) {
				animationTime_ = std::fmod(animationTime_, animation_->duration);
			} else {
				animationTime_ = std::min(animationTime_, animation_->duration);
			}
		}

		const auto& rootNode = model_->GetModelData().rootnode;
		const Animation::NodeAnimation* nodeAnimation = nullptr;
		auto findNodeAnimation = [&](const auto& node, const auto& nodeAnimations, auto&& self) -> const Animation::NodeAnimation* {
			auto it = nodeAnimations.find(node.name);
			if (it != nodeAnimations.end()) {
				return &it->second;
			}
			for (const auto& child : node.childlen) {
				if (const Animation::NodeAnimation* found = self(child, nodeAnimations, self)) {
					return found;
				}
			}
			return nullptr;
		};
		nodeAnimation = findNodeAnimation(rootNode, animation_->nodeAnimations, findNodeAnimation);
		if (nodeAnimation) {
			Vector3 translate = nodeAnimation->translate.keyframes.empty() ? Vector3{0.0f, 0.0f, 0.0f} : Animation::CalculateValue(nodeAnimation->translate, animationTime_);
			Vector4 rotate = nodeAnimation->rotation.keyframes.empty() ? Vector4{0.0f, 0.0f, 0.0f, 1.0f} : Animation::CalculateValue(nodeAnimation->rotation, animationTime_);
			Vector3 scale = nodeAnimation->scale.keyframes.empty() ? Vector3{1.0f, 1.0f, 1.0f} : Animation::CalculateValue(nodeAnimation->scale, animationTime_);
			localMatrix = Function::MakeAffineMatrix(scale, rotate, translate);
		}
	}

	if (!isUseSetWorld) {
		worldMatrix = Function::Multiply(localMatrix, Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate));
	} else {
		worldMatrix = Function::Multiply(localMatrix, worldMatrix);
	}

	worldViewProjectionMatrix = Function::Multiply(localMatrix, Function::Multiply(Function::Multiply(worldMatrix, camera_->GetViewMatrix()), camera_->GetProjectionMatrix()));
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
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(6, Object3dCommon::GetInstance()->GetSpotLightCountResource()->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(7, Object3dCommon::GetInstance()->GetAreaLightCountResource()->GetGPUVirtualAddress());
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
void Object3d::SetEnvironmentCoefficient(float coefficient) {
	if (model_) {
		model_->SetEnvironmentCoefficient(coefficient);
	}
}
void Object3d::CreateResources() {
	transformResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
	cameraResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(CameraForGpu));
}