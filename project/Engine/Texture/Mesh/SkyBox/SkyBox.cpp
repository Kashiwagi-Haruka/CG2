#include "SkyBox.h"

#include "Camera.h"
#include "Function.h"
#include "Object3d/Object3dCommon.h"
#include "TextureManager.h"
#include "WinApp.h"
#include <cstring>
#include "SrvManager/SrvManager.h"
#include "DirectXCommon.h"

SkyBox::SkyBox() {
	transform_.scale = {100.0f, 100.0f, 100.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {0.0f, 2.5f, 0.0f};
}

SkyBox::~SkyBox() = default;

void SkyBox::Initialize() {
	vertices_.resize(24);
	indices_ = {
	    0,  2,  1,  2,  3,  1,  // right (inside)
	    4,  6,  5,  6,  7,  5,  // left
	    8,  10, 9,  10, 11, 9,  // front
	    12, 14, 13, 14, 15, 13, // back
	    16, 18, 17, 18, 19, 17, // top
	    20, 22, 21, 22, 23, 21, // bottom
	};

	// right
	vertices_[0].position = {1.0f, 1.0f, 1.0f, 1.0f};
	vertices_[1].position = {1.0f, 1.0f, -1.0f, 1.0f};
	vertices_[2].position = {1.0f, -1.0f, 1.0f, 1.0f};
	vertices_[3].position = {1.0f, -1.0f, -1.0f, 1.0f};
	// left
	vertices_[4].position = {-1.0f, 1.0f, -1.0f, 1.0f};
	vertices_[5].position = {-1.0f, 1.0f, 1.0f, 1.0f};
	vertices_[6].position = {-1.0f, -1.0f, -1.0f, 1.0f};
	vertices_[7].position = {-1.0f, -1.0f, 1.0f, 1.0f};
	// front
	vertices_[8].position = {-1.0f, 1.0f, 1.0f, 1.0f};
	vertices_[9].position = {1.0f, 1.0f, 1.0f, 1.0f};
	vertices_[10].position = {-1.0f, -1.0f, 1.0f, 1.0f};
	vertices_[11].position = {1.0f, -1.0f, 1.0f, 1.0f};
	// back
	vertices_[12].position = {1.0f, 1.0f, -1.0f, 1.0f};
	vertices_[13].position = {-1.0f, 1.0f, -1.0f, 1.0f};
	vertices_[14].position = {1.0f, -1.0f, -1.0f, 1.0f};
	vertices_[15].position = {-1.0f, -1.0f, -1.0f, 1.0f};
	// top
	vertices_[16].position = {-1.0f, 1.0f, -1.0f, 1.0f};
	vertices_[17].position = {1.0f, 1.0f, -1.0f, 1.0f};
	vertices_[18].position = {-1.0f, 1.0f, 1.0f, 1.0f};
	vertices_[19].position = {1.0f, 1.0f, 1.0f, 1.0f};
	// bottom
	vertices_[20].position = {-1.0f, -1.0f, 1.0f, 1.0f};
	vertices_[21].position = {1.0f, -1.0f, 1.0f, 1.0f};
	vertices_[22].position = {-1.0f, -1.0f, -1.0f, 1.0f};
	vertices_[23].position = {1.0f, -1.0f, -1.0f, 1.0f};

	for (auto& v : vertices_) {
		v.texcoord = {0.0f, 0.0f};
		v.normal = {0.0f, 1.0f, 0.0f};
	}

	vertexResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * vertices_.size());
	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices_.data(), sizeof(VertexData) * vertices_.size());
	vertexResource_->Unmap(0, nullptr);
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices_.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	indexResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * indices_.size());
	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	std::memcpy(indexData, indices_.data(), sizeof(uint32_t) * indices_.size());
	indexResource_->Unmap(0, nullptr);
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices_.size());
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	transformResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
	cameraResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(CameraForGpu));
	const UINT aligned = (sizeof(Material) + 0xFF) & ~0xFF;
	materialResource_ = Object3dCommon::GetInstance()->CreateBufferResource(aligned);
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = {1, 1, 1, 1};
	materialData_->enableLighting = 0;
	materialData_->shininess = 0.0f;
	materialData_->environmentCoefficient = 0.0f;
	materialData_->uvTransform = Function::MakeIdentity4x4();
	materialResource_->Unmap(0, nullptr);

	SetDDSTexture("Resources/SkyBox/rostock_laage_airport_4k.dds");
}

void SkyBox::SetCamera(Camera* camera) { camera_ = camera; }

void SkyBox::SetDDSTexture(const std::string& filePath) {
	TextureManager::GetInstance()->LoadTextureName(filePath);
	textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByfilePath(filePath);
}

void SkyBox::SetScale(const Vector3& scale) { transform_.scale = scale; }
void SkyBox::SetRotation(const Vector3& rotation) { transform_.rotate = rotation; }

void SkyBox::Update() {
	Camera* activeCamera = camera_ ? camera_ : Object3dCommon::GetInstance()->GetDefaultCamera();
	const Vector3 skyboxTranslate = activeCamera ? activeCamera->GetWorldTranslate() : transform_.translate;
	const Matrix4x4 world = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, skyboxTranslate);
	const Matrix4x4 wvp = activeCamera ? Function::Multiply(Function::Multiply(world, activeCamera->GetViewMatrix()), activeCamera->GetProjectionMatrix()) : world;

	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	transformationMatrixData_->WVP = wvp;
	transformationMatrixData_->World = world;
	transformationMatrixData_->DirectionalLightWVP = Function::Multiply(world, Object3dCommon::GetInstance()->GetDirectionalLightViewProjectionMatrix());
	transformationMatrixData_->PointLightWVP = Function::Multiply(world, Object3dCommon::GetInstance()->GetPointLightViewProjectionMatrix());
	transformationMatrixData_->SpotLightWVP = Function::Multiply(world, Object3dCommon::GetInstance()->GetSpotLightViewProjectionMatrix());
	transformationMatrixData_->AreaLightWVP = Function::Multiply(world, Object3dCommon::GetInstance()->GetAreaLightViewProjectionMatrix());
	transformationMatrixData_->WorldInverseTranspose = Function::Inverse(world);
	transformResource_->Unmap(0, nullptr);

	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));
	cameraData_->worldPosition = activeCamera ? activeCamera->GetWorldTranslate() : Vector3{0, 0, 0};
	cameraData_->screenSize = {static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight)};
	cameraData_->fullscreenGrayscaleEnabled = 0;
	cameraData_->fullscreenSepiaEnabled = 0;
	cameraResource_->Unmap(0, nullptr);
}

void SkyBox::Draw() {
	ID3D12DescriptorHeap* descriptorHeaps[] = {TextureManager::GetInstance()->GetSrvManager()->GetDescriptorHeap().Get()};
	auto* cmd = Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList();
	cmd->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	cmd->IASetVertexBuffers(0, 1, &vertexBufferView_);
	cmd->IASetIndexBuffer(&indexBufferView_);
	cmd->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	cmd->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress());
	cmd->SetGraphicsRootConstantBufferView(4, cameraResource_->GetGPUVirtualAddress());
	cmd->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex_));
	cmd->DrawIndexedInstanced(static_cast<UINT>(indices_.size()), 1, 0, 0, 0);
}