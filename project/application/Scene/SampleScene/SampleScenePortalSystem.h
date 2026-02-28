#pragma once

#include "Camera.h"
#include "Object3d/Object3d.h"
#include "Primitive/Primitive.h"
#include "RenderTexture2D.h"
#include "Transform.h"
#include "Vector2.h"
#include "Vector3.h"
#include <functional>
#include <memory>

class SampleScenePortalSystem {
public:
	SampleScenePortalSystem();
	void Initialize(Camera* mainCamera, const Transform& portalATransform, const Transform& portalBTransform);
	void Update(Camera* mainCamera, float ringUvRotation);
	void RenderPortalTextures(const std::function<void(Camera*)>& drawSceneWithoutPortals);

	void SetCamera(Camera* camera);
	void UpdateCameraMatrices();
	void DrawPortals();
	void DrawRings();

private:
	void UpdatePortalCamera(const Transform& destinationPortal, Camera* outCamera);

	std::unique_ptr<Primitive> portalA_ = nullptr;
	std::unique_ptr<Primitive> portalB_ = nullptr;
	std::unique_ptr<Primitive> portalRingA_ = nullptr;
	std::unique_ptr<Primitive> portalRingB_ = nullptr;
	std::unique_ptr<Camera> portalTextureCameraA_ = nullptr;
	std::unique_ptr<Camera> portalTextureCameraB_ = nullptr;
	std::unique_ptr<RenderTexture2D> portalRenderTextureA_ = nullptr;
	std::unique_ptr<RenderTexture2D> portalRenderTextureB_ = nullptr;
	Camera* sceneCamera_ = nullptr;
};