#include "SampleScenePortalSystem.h"
#include "Function.h"
#include "Object3d/Object3dCommon.h"
#include "WinApp.h"
#include <cmath>
#include <numbers>
#include "Engine/Log/Logger.h"

SampleScenePortalSystem::SampleScenePortalSystem() {
	portalA_ = std::make_unique<Primitive>();
	portalB_ = std::make_unique<Primitive>();
	portalRingA_ = std::make_unique<Primitive>();
	portalRingB_ = std::make_unique<Primitive>();
	portalTextureCameraA_ = std::make_unique<Camera>();
	portalTextureCameraB_ = std::make_unique<Camera>();
}

void SampleScenePortalSystem::Initialize(Camera* mainCamera, const Transform& portalATransform, const Transform& portalBTransform) {
	portalA_->Initialize(Primitive::Circle, 48);
	portalA_->SetCamera(mainCamera);
	portalA_->SetColor({0.3f, 0.7f, 1.0f, 1.0f});
	portalA_->SetEnableLighting(false);
	portalB_->Initialize(Primitive::Circle, 48);
	portalB_->SetCamera(mainCamera);
	portalB_->SetEnableLighting(false);

	portalRingA_->Initialize(Primitive::Ring, "Resources/TD3_3102/2d/ring.png", 48);
	portalRingA_->SetCamera(mainCamera);
	portalRingA_->SetEnableLighting(false);
	portalRingA_->SetColor({0.3f, 0.7f, 1.0f, 1.0f});
	portalRingB_->Initialize(Primitive::Ring, "Resources/TD3_3102/2d/ring.png", 48);
	portalRingB_->SetCamera(mainCamera);
	portalRingB_->SetEnableLighting(false);
	portalRingB_->SetColor({1.0f, 0.55f, 0.1f, 1.0f});

	portalA_->SetTransform(portalATransform);
	portalB_->SetTransform(portalBTransform);
	portalRingA_->SetTransform(portalATransform);
	portalRingB_->SetTransform(portalBTransform);

	portalRenderTextureA_ = std::make_unique<RenderTexture2D>();
	portalRenderTextureA_->Initialize(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, {0.05f, 0.05f, 0.1f, 1.0f});
	if (portalRenderTextureA_->IsReady()) {
		portalB_->SetTextureIndex(portalRenderTextureA_->GetSrvIndex());
		portalB_->SetSecondaryTextureIndex(portalRenderTextureA_->GetSrvIndex());
	}
	portalRenderTextureB_ = std::make_unique<RenderTexture2D>();
	portalRenderTextureB_->Initialize(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, {0.05f, 0.05f, 0.1f, 1.0f});
	if (portalRenderTextureB_->IsReady()) {
		portalA_->SetTextureIndex(portalRenderTextureB_->GetSrvIndex());
		portalA_->SetSecondaryTextureIndex(portalRenderTextureB_->GetSrvIndex());
	}
	portalA_->SetPortalProjectionEnabled(true);
	portalB_->SetPortalProjectionEnabled(true);
}

void SampleScenePortalSystem::Update(Camera* mainCamera, float ringUvRotation) {
	sceneCamera_ = mainCamera;
	portalA_->Update();
	portalB_->Update();
	const Transform portalATransform = portalA_->GetTransform();
	const Transform portalBTransform = portalB_->GetTransform();
	portalRingA_->SetTransform(portalATransform);
	portalRingB_->SetTransform(portalBTransform);
	portalRingA_->SetUvTransform(Vector3(1, 1, 1), Vector3(0, 0, ringUvRotation), Vector3(0, 0, 0), Vector2(0.5f, 0.5f));
	portalRingB_->SetUvTransform(Vector3(1, 1, 1), Vector3(0, 0, -ringUvRotation), Vector3(0, 0, 0), Vector2(0.5f, 0.5f));
	portalRingA_->Update();
	portalRingB_->Update();
}
void SampleScenePortalSystem::SetPortalTransforms(const Transform& portalATransform, const Transform& portalBTransform) {
	portalA_->SetTransform(portalATransform);
	portalB_->SetTransform(portalBTransform);
	portalRingA_->SetTransform(portalATransform);
	portalRingB_->SetTransform(portalBTransform);
}
void SampleScenePortalSystem::RenderPortalTextures(const std::function<void(Camera*)>& drawSceneWithoutPortals, const std::function<void(Camera*)>& drawPortals) {
	if (!sceneCamera_ || !portalTextureCameraA_ || !portalTextureCameraB_) {
		return;
	}

	const Transform portalATransform = portalA_->GetTransform();
	const Transform portalBTransform = portalB_->GetTransform();
	constexpr int kPortalRecursionPassCount = 2;

	for (int pass = 0; pass < kPortalRecursionPassCount; ++pass) {
		if (portalRenderTextureA_ && portalRenderTextureA_->IsReady()) {
			UpdatePortalCamera(portalBTransform, portalATransform, portalTextureCameraA_.get());
			portalRenderTextureA_->BeginRender();
			drawSceneWithoutPortals(portalTextureCameraA_.get());
			drawPortals(portalTextureCameraA_.get());
			portalRenderTextureA_->TransitionToShaderResource();
		}
		if (portalRenderTextureB_ && portalRenderTextureB_->IsReady()) {
			UpdatePortalCamera(portalATransform, portalBTransform, portalTextureCameraB_.get());
			portalRenderTextureB_->BeginRender();
			drawSceneWithoutPortals(portalTextureCameraB_.get());
			drawPortals(portalTextureCameraB_.get());
			portalRenderTextureB_->TransitionToShaderResource();
		}
	}

	portalA_->SetPortalProjectionMatrices(
	    portalTextureCameraB_->GetViewProjectionMatrix(), portalTextureCameraB_->GetViewProjectionMatrix(), portalTextureCameraB_->GetWorldMatrix(), portalTextureCameraB_->GetWorldMatrix());
	portalB_->SetPortalProjectionMatrices(
	    portalTextureCameraA_->GetViewProjectionMatrix(), portalTextureCameraA_->GetViewProjectionMatrix(), portalTextureCameraA_->GetWorldMatrix(), portalTextureCameraA_->GetWorldMatrix());
}

void SampleScenePortalSystem::SetCamera(Camera* camera) {
	sceneCamera_ = camera;
	portalA_->SetCamera(camera);
	portalB_->SetCamera(camera);
	portalRingA_->SetCamera(camera);
	portalRingB_->SetCamera(camera);
}

void SampleScenePortalSystem::UpdateCameraMatrices() {
	portalA_->UpdateCameraMatrices();
	portalB_->UpdateCameraMatrices();
	portalRingA_->UpdateCameraMatrices();
	portalRingB_->UpdateCameraMatrices();
}

void SampleScenePortalSystem::DrawPortals() {
	Object3dCommon::GetInstance()->DrawCommonPortal();
	portalA_->Draw();
	portalB_->Draw();
}

void SampleScenePortalSystem::DrawRings() {
	Object3dCommon::GetInstance()->DrawCommonNoCull();
	portalRingA_->Draw();
	portalRingB_->Draw();
}

void SampleScenePortalSystem::SetPortalCameraPositionOffset(const Vector3& offset) { portalCameraPositionOffset_ = offset; }

void SampleScenePortalSystem::SetPortalCameraRotationOffset(const Vector3& offset) { portalCameraRotationOffset_ = offset; }

const Vector3& SampleScenePortalSystem::GetPortalCameraPositionOffset() const { return portalCameraPositionOffset_; }

const Vector3& SampleScenePortalSystem::GetPortalCameraRotationOffset() const { return portalCameraRotationOffset_; }

void SampleScenePortalSystem::UpdatePortalCamera(const Transform& sourcePortal, const Transform& destinationPortal, Camera* outCamera) {
	if (!outCamera) {
		return;
	}

	const Matrix4x4 sourcePortalWorld = Function::MakeAffineMatrix(sourcePortal.scale, sourcePortal.rotate, sourcePortal.translate);
	const Matrix4x4 destinationPortalWorld = Function::MakeAffineMatrix(destinationPortal.scale, destinationPortal.rotate, destinationPortal.translate);
	const Matrix4x4 sourceInverse = Function::Inverse(sourcePortalWorld);

	// ローカル空間へ
	const Matrix4x4 cameraInSourceSpace = Function::Multiply(sceneCamera_->GetWorldMatrix(), sourceInverse);

	// destinationへ変換
	Matrix4x4 portalCameraWorld = Function::Multiply(cameraInSourceSpace, destinationPortalWorld);
	if (Function::LengthSquared(portalCameraRotationOffset_) > 0.0001f) {
		const Matrix4x4 rotationOffset = Function::Multiply(
		    Function::Multiply(Function::MakeRotateXMatrix(portalCameraRotationOffset_.x), Function::MakeRotateYMatrix(portalCameraRotationOffset_.y)),
		    Function::MakeRotateZMatrix(portalCameraRotationOffset_.z));
		portalCameraWorld = Function::Multiply(rotationOffset, portalCameraWorld);
	}
	portalCameraWorld.m[3][0] += portalCameraPositionOffset_.x;
	portalCameraWorld.m[3][1] += portalCameraPositionOffset_.y;
	portalCameraWorld.m[3][2] += portalCameraPositionOffset_.z;

	Vector3 destinationForward = {destinationPortalWorld.m[2][0], destinationPortalWorld.m[2][1], destinationPortalWorld.m[2][2]};
	if (Function::LengthSquared(destinationForward) < 0.0001f) {
		destinationForward = {0.0f, 0.0f, 1.0f};
	}
	destinationForward = Function::Normalize(destinationForward);
	portalCameraWorld.m[3][0] += destinationForward.x * 0.05f;
	portalCameraWorld.m[3][1] += destinationForward.y * 0.05f;
	portalCameraWorld.m[3][2] += destinationForward.z * 0.05f;

	// View更新
	const Matrix4x4 portalViewMatrix = Function::Inverse(portalCameraWorld);

	// ★ Worldも更新する
	outCamera->SetWorldMatrix(portalCameraWorld);

	// ViewProjection更新
	outCamera->SetViewProjectionMatrix(portalViewMatrix, sceneCamera_->GetProjectionMatrix());
	outCamera->SetFovY(1.2f);
}