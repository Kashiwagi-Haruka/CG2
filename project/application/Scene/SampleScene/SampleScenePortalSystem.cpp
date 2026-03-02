#include "SampleScenePortalSystem.h"
#include "Function.h"
#include "Object3d/Object3dCommon.h"
#include "WinApp.h"
#include <cmath>
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

void SampleScenePortalSystem::UpdatePortalCamera(const Transform& sourcePortal, const Transform& destinationPortal, Camera* outCamera) {
	if (!outCamera) {
		return;
	}

	const Matrix4x4 sourcePortalWorld = Function::MakeAffineMatrix(sourcePortal.scale, sourcePortal.rotate, sourcePortal.translate);
	const Matrix4x4 destinationPortalWorld = Function::MakeAffineMatrix(destinationPortal.scale, destinationPortal.rotate, destinationPortal.translate);
	const Matrix4x4 halfTurn = Function::MakeRotateYMatrix(std::numbers::pi_v<float>);

	// メインカメラを sourcePortal 空間へ持っていき、180度反転して destinationPortal 空間へ写像する。
	const Matrix4x4 sourcePortalInverse = Function::Inverse(sourcePortalWorld);
	const Matrix4x4 portalMapping = Function::Multiply(Function::Multiply(sourcePortalInverse, halfTurn), destinationPortalWorld);
	const Matrix4x4 cameraWorld = sceneCamera_->GetWorldMatrix();

	Vector3 cameraPosition = {cameraWorld.m[3][0], cameraWorld.m[3][1], cameraWorld.m[3][2]};
	Vector3 cameraRight = Function::Normalize({cameraWorld.m[0][0], cameraWorld.m[0][1], cameraWorld.m[0][2]});
	Vector3 cameraUp = Function::Normalize({cameraWorld.m[1][0], cameraWorld.m[1][1], cameraWorld.m[1][2]});
	Vector3 cameraForward = Function::Normalize({cameraWorld.m[2][0], cameraWorld.m[2][1], cameraWorld.m[2][2]});

	Vector3 mappedPosition = Function::TransformVM(cameraPosition, portalMapping);
	Vector3 mappedRightPoint = Function::TransformVM(cameraPosition + cameraRight, portalMapping);
	Vector3 mappedUpPoint = Function::TransformVM(cameraPosition + cameraUp, portalMapping);
	Vector3 mappedForwardPoint = Function::TransformVM(cameraPosition + cameraForward, portalMapping);

	Vector3 mappedRight = Function::Normalize(mappedRightPoint - mappedPosition);
	Vector3 mappedUp = Function::Normalize(mappedUpPoint - mappedPosition);
	Vector3 mappedForward = Function::Normalize(mappedForwardPoint - mappedPosition);

	Matrix4x4 portalCameraWorld = Function::MakeIdentity4x4();
	portalCameraWorld.m[0][0] = mappedRight.x;
	portalCameraWorld.m[0][1] = mappedRight.y;
	portalCameraWorld.m[0][2] = mappedRight.z;
	portalCameraWorld.m[1][0] = mappedUp.x;
	portalCameraWorld.m[1][1] = mappedUp.y;
	portalCameraWorld.m[1][2] = mappedUp.z;
	portalCameraWorld.m[2][0] = mappedForward.x;
	portalCameraWorld.m[2][1] = mappedForward.y;
	portalCameraWorld.m[2][2] = mappedForward.z;
	portalCameraWorld.m[3][0] = mappedPosition.x;
	portalCameraWorld.m[3][1] = mappedPosition.y;
	portalCameraWorld.m[3][2] = mappedPosition.z;

	Vector3 destinationForward = {destinationPortalWorld.m[2][0], destinationPortalWorld.m[2][1], destinationPortalWorld.m[2][2]};
	if (Function::LengthSquared(destinationForward) < 0.0001f) {
		destinationForward = {0.0f, 0.0f, 1.0f};
	}
	destinationForward = Function::Normalize(destinationForward);
	portalCameraWorld.m[3][0] += destinationForward.x * 0.05f;
	portalCameraWorld.m[3][1] += destinationForward.y * 0.05f;
	portalCameraWorld.m[3][2] += destinationForward.z * 0.05f;

	const Matrix4x4 portalViewMatrix = Function::Inverse(portalCameraWorld);
	outCamera->SetViewProjectionMatrix(portalViewMatrix, sceneCamera_->GetProjectionMatrix());
}