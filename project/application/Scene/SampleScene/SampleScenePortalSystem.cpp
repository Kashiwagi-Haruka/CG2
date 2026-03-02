#include "SampleScenePortalSystem.h"

#include "Function.h"
#include "Object3d/Object3dCommon.h"
#include "WinApp.h"

#include <numbers>

SampleScenePortalSystem::SampleScenePortalSystem() {
	portalA_ = std::make_unique<Primitive>();
	portalB_ = std::make_unique<Primitive>();
	portalRingA_ = std::make_unique<Primitive>();
	portalRingB_ = std::make_unique<Primitive>();
	portalTextureCameraA_ = std::make_unique<Camera>();
	portalTextureCameraB_ = std::make_unique<Camera>();
}

void SampleScenePortalSystem::Initialize(Camera* mainCamera, const Transform& portalATransform, const Transform& portalBTransform) {
	sceneCamera_ = mainCamera;

	portalA_->Initialize(Primitive::Circle, 64);
	portalB_->Initialize(Primitive::Circle, 64);
	portalA_->SetEnableLighting(false);
	portalB_->SetEnableLighting(false);
	portalA_->SetColor({0.95f, 0.95f, 1.0f, 1.0f});
	portalB_->SetColor({0.95f, 0.95f, 1.0f, 1.0f});
	portalA_->SetPortalProjectionEnabled(true);
	portalB_->SetPortalProjectionEnabled(true);

	portalRingA_->Initialize(Primitive::Ring, "Resources/TD3_3102/2d/ring.png", 64);
	portalRingB_->Initialize(Primitive::Ring, "Resources/TD3_3102/2d/ring.png", 64);
	portalRingA_->SetEnableLighting(false);
	portalRingB_->SetEnableLighting(false);
	portalRingA_->SetColor({0.2f, 0.7f, 1.0f, 1.0f});
	portalRingB_->SetColor({1.0f, 0.55f, 0.1f, 1.0f});

	SetCamera(mainCamera);
	SetPortalTransforms(portalATransform, portalBTransform);

	portalRenderTextureA_ = std::make_unique<RenderTexture2D>();
	portalRenderTextureA_->Initialize(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, {0.02f, 0.02f, 0.05f, 1.0f});
	if (portalRenderTextureA_->IsReady()) {
		portalB_->SetTextureIndex(portalRenderTextureA_->GetSrvIndex());
		portalB_->SetSecondaryTextureIndex(portalRenderTextureA_->GetSrvIndex());
	}

	portalRenderTextureB_ = std::make_unique<RenderTexture2D>();
	portalRenderTextureB_->Initialize(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, {0.02f, 0.02f, 0.05f, 1.0f});
	if (portalRenderTextureB_->IsReady()) {
		portalA_->SetTextureIndex(portalRenderTextureB_->GetSrvIndex());
		portalA_->SetSecondaryTextureIndex(portalRenderTextureB_->GetSrvIndex());
	}
}

void SampleScenePortalSystem::Update(Camera* mainCamera, float ringUvRotation) {
	sceneCamera_ = mainCamera;

	portalA_->Update();
	portalB_->Update();

	const Transform portalATransform = portalA_->GetTransform();
	const Transform portalBTransform = portalB_->GetTransform();
	portalRingA_->SetTransform(portalATransform);
	portalRingB_->SetTransform(portalBTransform);
	portalRingA_->SetUvTransform(Vector3{1.0f, 1.0f, 1.0f}, Vector3{0.0f, 0.0f, ringUvRotation}, Vector3{0.0f, 0.0f, 0.0f}, Vector2{0.5f, 0.5f});
	portalRingB_->SetUvTransform(Vector3{1.0f, 1.0f, 1.0f}, Vector3{0.0f, 0.0f, -ringUvRotation}, Vector3{0.0f, 0.0f, 0.0f}, Vector2{0.5f, 0.5f});
	portalRingA_->Update();
	portalRingB_->Update();
}

void SampleScenePortalSystem::RenderPortalTextures(const std::function<void(Camera*)>& drawSceneWithoutPortals, const std::function<void(Camera*)>& drawPortals) {
	if (!sceneCamera_) {
		return;
	}

	const Transform portalATransform = portalA_->GetTransform();
	const Transform portalBTransform = portalB_->GetTransform();

	for (int pass = 0; pass < recursionPassCount_; ++pass) {
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

	portalA_->Update();
	portalB_->Update();
	portalA_->UpdateCameraMatrices();
	portalB_->UpdateCameraMatrices();
}

void SampleScenePortalSystem::SetCamera(Camera* camera) {
	sceneCamera_ = camera;
	portalA_->SetCamera(camera);
	portalB_->SetCamera(camera);
	portalRingA_->SetCamera(camera);
	portalRingB_->SetCamera(camera);
}

void SampleScenePortalSystem::SetPortalTransforms(const Transform& portalATransform, const Transform& portalBTransform) {
	portalA_->SetTransform(portalATransform);
	portalB_->SetTransform(portalBTransform);
	portalRingA_->SetTransform(portalATransform);
	portalRingB_->SetTransform(portalBTransform);
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
	if (!sceneCamera_ || !outCamera) {
		return;
	}

	const Matrix4x4 sourceWorld = Function::MakeAffineMatrix(sourcePortal.scale, sourcePortal.rotate, sourcePortal.translate);
	const Matrix4x4 destinationWorld = Function::MakeAffineMatrix(destinationPortal.scale, destinationPortal.rotate, destinationPortal.translate);
	const Matrix4x4 sourceInverse = Function::Inverse(sourceWorld);
	const Matrix4x4 cameraInSourceSpace = Function::Multiply(sourceInverse, sceneCamera_->GetWorldMatrix());

	const Matrix4x4 halfTurn = Function::MakeRotateYMatrix(std::numbers::pi_v<float>);
	Matrix4x4 portalCameraWorld = Function::Multiply(destinationWorld, Function::Multiply(halfTurn, cameraInSourceSpace));

	if (Function::LengthSquared(portalCameraRotationOffset_) > 0.000001f) {
		const Matrix4x4 rotationOffset = Function::Multiply(
		    Function::Multiply(Function::MakeRotateXMatrix(portalCameraRotationOffset_.x), Function::MakeRotateYMatrix(portalCameraRotationOffset_.y)),
		    Function::MakeRotateZMatrix(portalCameraRotationOffset_.z));
		portalCameraWorld = Function::Multiply(rotationOffset, portalCameraWorld);
	}

	portalCameraWorld.m[3][0] += portalCameraPositionOffset_.x;
	portalCameraWorld.m[3][1] += portalCameraPositionOffset_.y;
	portalCameraWorld.m[3][2] += portalCameraPositionOffset_.z;

	Vector3 destinationForward = {destinationWorld.m[2][0], destinationWorld.m[2][1], destinationWorld.m[2][2]};
	if (Function::LengthSquared(destinationForward) < 0.000001f) {
		destinationForward = {0.0f, 0.0f, 1.0f};
	}
	destinationForward = Function::Normalize(destinationForward);
	portalCameraWorld.m[3][0] += destinationForward.x * nearPlanePush_;
	portalCameraWorld.m[3][1] += destinationForward.y * nearPlanePush_;
	portalCameraWorld.m[3][2] += destinationForward.z * nearPlanePush_;

	outCamera->SetWorldMatrix(portalCameraWorld);
	const Matrix4x4 view = Function::Inverse(portalCameraWorld);
	const Matrix4x4 projection = Function::MakePerspectiveFovMatrix(sceneCamera_->GetFovY(), sceneCamera_->GetAspectRatio(), sceneCamera_->GetNearZ(), sceneCamera_->GetFarZ());
	outCamera->SetViewProjectionMatrix(view, projection);
	outCamera->Update();
}