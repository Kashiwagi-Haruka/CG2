#include "SampleScenePortalSystem.h"
#include "Object3d/Object3dCommon.h"
#include "Function.h"
#include "WinApp.h"
#include <cmath>
#include <numbers>

SampleScenePortalSystem::SampleScenePortalSystem() {
	portalA_ = std::make_unique<Primitive>();
	portalB_ = std::make_unique<Primitive>();
	portalRingA_ = std::make_unique<Primitive>();
	portalRingB_ = std::make_unique<Primitive>();
	portalCameraFromA_ = std::make_unique<Camera>();
	portalCameraFromB_ = std::make_unique<Camera>();
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
	}
	portalRenderTextureB_ = std::make_unique<RenderTexture2D>();
	portalRenderTextureB_->Initialize(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, {0.05f, 0.05f, 0.1f, 1.0f});
	if (portalRenderTextureB_->IsReady()) {
		portalA_->SetTextureIndex(portalRenderTextureB_->GetSrvIndex());
	}
}

void SampleScenePortalSystem::Update(Camera* mainCamera, float ringUvRotation) {
	portalA_->Update();
	portalB_->Update();
	const Transform portalATransform = portalA_->GetTransform();
	const Transform portalBTransform = portalB_->GetTransform();
	UpdatePortalCamera(mainCamera, portalATransform, portalBTransform, portalCameraFromA_.get());
	UpdatePortalCamera(mainCamera, portalBTransform, portalATransform, portalCameraFromB_.get());
	portalRingA_->SetTransform(portalATransform);
	portalRingB_->SetTransform(portalBTransform);
	portalRingA_->SetUvTransform(Vector3(1, 1, 1), Vector3(0, 0, ringUvRotation), Vector3(0, 0, 0), Vector2(0.5f, 0.5f));
	portalRingB_->SetUvTransform(Vector3(1, 1, 1), Vector3(0, 0, -ringUvRotation), Vector3(0, 0, 0), Vector2(0.5f, 0.5f));
	portalRingA_->Update();
	portalRingB_->Update();
}

void SampleScenePortalSystem::RenderPortalTextures(const std::function<void(Camera*)>& drawSceneWithoutPortals) {
	if (portalRenderTextureA_ && portalRenderTextureA_->IsReady()) {
		portalRenderTextureA_->BeginRender();
		drawSceneWithoutPortals(portalCameraFromA_.get());
		portalRenderTextureA_->TransitionToShaderResource();
	}
	if (portalRenderTextureB_ && portalRenderTextureB_->IsReady()) {
		portalRenderTextureB_->BeginRender();
		drawSceneWithoutPortals(portalCameraFromB_.get());
		portalRenderTextureB_->TransitionToShaderResource();
	}
}

void SampleScenePortalSystem::SetCamera(Camera* camera) {
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
	Object3dCommon::GetInstance()->DrawCommonNoCull();
	portalA_->Draw();
	portalB_->Draw();
}

void SampleScenePortalSystem::DrawRings() {
	Object3dCommon::GetInstance()->DrawCommonNoCull();
	portalRingA_->Draw();
	portalRingB_->Draw();
}

void SampleScenePortalSystem::UpdatePortalCamera(Camera* mainCamera, const Transform& sourcePortal, const Transform& destinationPortal, Camera* outCamera) {
	if (!mainCamera || !outCamera) {
		return;
	}

	const Matrix4x4 mainCameraWorld = mainCamera->GetWorldMatrix();
	const Matrix4x4 mainCameraView = mainCamera->GetViewMatrix();
	const Matrix4x4 sourcePortalWorld = Function::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, sourcePortal.rotate, sourcePortal.translate);
	const Matrix4x4 destinationPortalWorld = Function::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, destinationPortal.rotate, destinationPortal.translate);
	const Matrix4x4 sourcePortalInverse = Function::Inverse(sourcePortalWorld);
	const Matrix4x4 halfTurn = Function::MakeRotateYMatrix(std::numbers::pi_v<float>);

	Matrix4x4 portalCameraWorld = Function::Multiply(Function::Multiply(Function::Multiply(mainCameraWorld, sourcePortalInverse), halfTurn), destinationPortalWorld);

	Vector3 destinationForward = {destinationPortalWorld.m[2][0], destinationPortalWorld.m[2][1], destinationPortalWorld.m[2][2]};
	if (Function::LengthSquared(destinationForward) < 0.0001f) {
		destinationForward = {0.0f, 0.0f, 1.0f};
	}
	destinationForward = Function::Normalize(destinationForward);
	portalCameraWorld.m[3][0] += destinationForward.x * 0.05f;
	portalCameraWorld.m[3][1] += destinationForward.y * 0.05f;
	portalCameraWorld.m[3][2] += destinationForward.z * 0.05f;

	Matrix4x4 portalViewMatrix = Function::Inverse(portalCameraWorld);

	float diffSum = 0.0f;
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			diffSum += std::abs(portalViewMatrix.m[row][column] - mainCameraView.m[row][column]);
		}
	}
	if (diffSum < 0.001f) {
		Vector3 destinationRight = {destinationPortalWorld.m[0][0], destinationPortalWorld.m[0][1], destinationPortalWorld.m[0][2]};
		if (Function::LengthSquared(destinationRight) < 0.0001f) {
			destinationRight = {1.0f, 0.0f, 0.0f};
		}
		destinationRight = Function::Normalize(destinationRight);
		portalCameraWorld.m[3][0] += destinationRight.x * 0.1f;
		portalCameraWorld.m[3][1] += destinationRight.y * 0.1f;
		portalCameraWorld.m[3][2] += destinationRight.z * 0.1f;
		portalViewMatrix = Function::Inverse(portalCameraWorld);
	}

	outCamera->SetViewProjectionMatrix(portalViewMatrix, mainCamera->GetProjectionMatrix());
}