#include "PortalMesh.h"

#include "Camera.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/base/DirectXCommon.h"
#include "Function.h"
#include "Object3d/Object3dCommon.h"
#include "SrvManager/SrvManager.h"
#include "WinApp.h"
#include <array>
#include <cmath>
#include <cstring>
#include <numbers>

void PortalMesh::Initialize(const std::string& texturePath) {
	textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByfilePath(texturePath);
	worldMatrix_ = Function::MakeIdentity4x4();

	constexpr uint32_t kPortalSegments = 32;
	constexpr uint32_t kPortalVertexCount = kPortalSegments + 1;
	constexpr uint32_t kPortalIndexCount = kPortalSegments * 3;
	float kPortalRadius = transform_.scale.x / 2.0f;

	std::array<VertexData, kPortalVertexCount> vertices{};
	std::array<uint32_t, kPortalIndexCount> indices{};

	vertices[0] = {
	    {0.0f, 0.0f, 0.0f, 1.0f},
        {0.5f, 0.5f},
        {0.0f, 0.0f, 1.0f}
    };
	for (uint32_t segment = 0; segment < kPortalSegments; ++segment) {
		const float angle = (2.0f * std::numbers::pi_v<float> * static_cast<float>(segment)) / static_cast<float>(kPortalSegments);
		const float x = std::cos(angle) * kPortalRadius;
		const float y = std::sin(angle) * kPortalRadius;
		vertices[segment + 1] = {
		    {x, y, 0.0f, 1.0f},
            {x / (kPortalRadius * 2.0f) + 0.5f, 0.5f - y / (kPortalRadius * 2.0f)},
            {0.0f, 0.0f, 1.0f}
        };

		const uint32_t nextSegmentIndex = (segment + 1) % kPortalSegments;
		const uint32_t indexOffset = segment * 3;
		indices[indexOffset + 0] = 0;
		indices[indexOffset + 1] = nextSegmentIndex + 1;
		indices[indexOffset + 2] = segment + 1;
	}
	indexCount_ = kPortalIndexCount;

	vertexResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * vertices.size());
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	VertexData* mappedVertices = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertices));
	std::memcpy(mappedVertices, vertices.data(), sizeof(VertexData) * vertices.size());
	vertexResource_->Unmap(0, nullptr);

	indexResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * indices.size());
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices.size());
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	uint32_t* mappedIndices = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedIndices));
	std::memcpy(mappedIndices, indices.data(), sizeof(uint32_t) * indices.size());
	indexResource_->Unmap(0, nullptr);

	transformResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
	objectCameraResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(CameraForGpu));
	textureCameraResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(PortalTextureCameraForGpu));
	const size_t alignedMaterialSize = (sizeof(Material) + 0xFF) & ~0xFF;
	materialResource_ = Object3dCommon::GetInstance()->CreateBufferResource(alignedMaterialSize);

	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = {1.0f, 1.0f, 1.0f, 1.0f};
	materialData_->enableLighting = false;
	materialData_->uvTransform = Function::MakeIdentity4x4();
	materialData_->shininess = 20.0f;
	materialData_->environmentCoefficient = 0.0f;
	materialData_->grayscaleEnabled = false;
	materialData_->sepiaEnabled = false;
	materialData_->distortionStrength = 0.1f;
	materialData_->distortionFalloff = 1.0f;
	materialResource_->Unmap(0, nullptr);
}

void PortalMesh::Update() {
	Camera* activeObjectCamera = Object3dCommon::GetInstance()->GetDefaultCamera();
	Camera* activeTextureCamera = textureCamera_;
	if (!activeObjectCamera) {
		return;
	}

	if (!useWorldMatrix_) {
		worldMatrix_ = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	}
	const bool hasTextureCamera = (activeTextureCamera != nullptr);
	const Matrix4x4 worldViewProjectionMatrix = Function::Multiply(Function::Multiply(worldMatrix_, activeObjectCamera->GetViewMatrix()), activeObjectCamera->GetProjectionMatrix());

	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix_;
	transformationMatrixData_->DirectionalLightWVP = Function::Multiply(worldMatrix_, Object3dCommon::GetInstance()->GetDirectionalLightViewProjectionMatrix());
	transformationMatrixData_->PointLightWVP = Function::Multiply(worldMatrix_, Object3dCommon::GetInstance()->GetPointLightViewProjectionMatrix());
	transformationMatrixData_->SpotLightWVP = Function::Multiply(worldMatrix_, Object3dCommon::GetInstance()->GetSpotLightViewProjectionMatrix());
	transformationMatrixData_->AreaLightWVP = Function::Multiply(worldMatrix_, Object3dCommon::GetInstance()->GetAreaLightViewProjectionMatrix());
	transformationMatrixData_->WorldInverseTranspose = Function::Inverse(worldMatrix_);
	transformResource_->Unmap(0, nullptr);

	textureCameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&textureCameraData_));
	textureCameraData_->textureViewProjection = hasTextureCamera ? activeTextureCamera->GetViewProjectionMatrix() : Function::MakeIdentity4x4();
	textureCameraData_->portalCameraWorld = hasTextureCamera ? activeTextureCamera->GetWorldMatrix() : Function::MakeIdentity4x4();
	textureCameraData_->textureWorldViewProjection =
	    hasTextureCamera ? Function::Multiply(Function::Multiply(worldMatrix_, activeTextureCamera->GetViewMatrix()), activeTextureCamera->GetProjectionMatrix()) : Function::MakeIdentity4x4();
	textureCameraData_->textureWorldPosition = hasTextureCamera ? activeTextureCamera->GetWorldTranslate() : Vector3{0.0f, 0.0f, 0.0f};
	textureCameraData_->usePortalProjection = hasTextureCamera ? 1 : 0;

	textureCameraResource_->Unmap(0, nullptr);

	objectCameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&objectCameraData_));
	objectCameraData_->worldPosition = activeObjectCamera->GetWorldTranslate();
	objectCameraData_->screenSize = {static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight)};
	objectCameraData_->fullscreenGrayscaleEnabled = Object3dCommon::GetInstance()->IsFullScreenGrayscaleEnabled() ? 1 : 0;
	objectCameraData_->fullscreenSepiaEnabled = Object3dCommon::GetInstance()->IsFullScreenSepiaEnabled() ? 1 : 0;
	objectCameraData_->usePortalProjection = 1;
	objectCameraResource_->Unmap(0, nullptr);
}

void PortalMesh::Draw() {
	ID3D12DescriptorHeap* descriptorHeaps[] = {TextureManager::GetInstance()->GetSrvManager()->GetDescriptorHeap().Get()};
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView_);
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, objectCameraResource_->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(5, textureCameraResource_->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex_));
	/*TextureManager::GetInstance()->GetSrvManager()->SetGraphicsRootDescriptorTable(8, Object3dCommon::GetInstance()->GetPointLightSrvIndex());
	TextureManager::GetInstance()->GetSrvManager()->SetGraphicsRootDescriptorTable(9, Object3dCommon::GetInstance()->GetSpotLightSrvIndex());
	TextureManager::GetInstance()->GetSrvManager()->SetGraphicsRootDescriptorTable(10, Object3dCommon::GetInstance()->GetAreaLightSrvIndex());*/
	if (!Object3dCommon::GetInstance()->IsShadowMapPassActive()) {
		TextureManager::GetInstance()->GetSrvManager()->SetGraphicsRootDescriptorTable(12, Object3dCommon::GetInstance()->GetDirectionalShadowMapSrvIndex());
		TextureManager::GetInstance()->GetSrvManager()->SetGraphicsRootDescriptorTable(13, Object3dCommon::GetInstance()->GetPointShadowMapSrvIndex());
		TextureManager::GetInstance()->GetSrvManager()->SetGraphicsRootDescriptorTable(14, Object3dCommon::GetInstance()->GetSpotShadowMapSrvIndex());
		TextureManager::GetInstance()->GetSrvManager()->SetGraphicsRootDescriptorTable(15, Object3dCommon::GetInstance()->GetAreaShadowMapSrvIndex());
	
	}
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(indexCount_, 1, 0, 0, 0);
}

void PortalMesh::SetUvTransform(const Matrix4x4& uvTransform) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->uvTransform = uvTransform;
	materialResource_->Unmap(0, nullptr);
}

void PortalMesh::SetColor(const Vector4& color) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = color;
	materialResource_->Unmap(0, nullptr);
}