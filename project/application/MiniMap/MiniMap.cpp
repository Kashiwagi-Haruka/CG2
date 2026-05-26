#include "MiniMap.h"

#include "Camera.h"
#include "Object3d/Object3d.h"
#include "Object3d/Object3dCommon.h"
#include "Portal/PortalMesh.h"
#include "Sprite/Sprite.h"
#include "Sprite/SpriteCommon.h"
#include "RenderTexture2D.h"
#include "TextureManager.h"
#include "WinApp.h"

#include <cmath>
#include <utility>

std::unique_ptr<MiniMap> MiniMap::instance_ = nullptr;

namespace {
float LengthXZ(const Vector3& v) { return std::sqrt((v.x * v.x) + (v.z * v.z)); }
} // namespace

MiniMap* MiniMap::GetInstance() {
	if (!instance_) {
		instance_ = std::make_unique<MiniMap>();
	}
	return instance_.get();
}

void MiniMap::Finalize() { instance_.reset(); }

void MiniMap::Initialize() {
	camera_ = std::make_unique<Camera>();
	camera_->SetRotate({1.5707963f, 0.0f, 0.0f});
	camera_->SetNearClip(0.1f);
	camera_->SetFarClip(500.0f);
	camera_->SetFovY(0.45f);
	camera_->SetAspectRatio(1.0f);

	miniMapCamera_ = std::make_unique<Camera>();
	miniMapCamera_->SetRotate({0.0f, 3.1415926f, 0.0f});
	miniMapCamera_->SetNearClip(0.1f);
	miniMapCamera_->SetFarClip(500.0f);
	miniMapCamera_->SetFovY(0.45f);
	miniMapCamera_->SetAspectRatio(1.0f);

	markerObject_ = std::make_unique<Object3d>();
	markerObject_->Initialize();
	markerObject_->SetModel("debugBox");
	markerObject_->SetCamera(camera_.get());
	markerObject_->SetEnableLighting(false);
	markerObject_->SetScale({0.2f, 0.2f, 0.2f});

	miniMapBackTextureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/white2x2.png");
	miniMapPlayerTextureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/MiniMapPlayer.png");
	miniMapPortal_ = std::make_unique<PortalMesh>();
	Transform miniMapPortalTransform{};
	miniMapPortalTransform.scale = {miniMapRadius_ * 2.2f, miniMapRadius_ * 2.2f, 1.0f};
	miniMapPortalTransform.rotate = {0.0f, 0.0f, 0.0f};
	miniMapPortalTransform.translate = {-1.2f, -0.8f, 1.5f};
	miniMapPortal_->SetTransform(miniMapPortalTransform);
	miniMapPortal_->Initialize("Resources/TD3_3102/2d/white2x2.png");
	miniMapPortal_->SetTextureCamera(miniMapCamera_.get());
	miniMapPortal_->SetColor({0.65f, 0.85f, 1.0f, 0.85f});

	miniMapPortalOutline_ = std::make_unique<PortalMesh>();
	miniMapPortalOutline_->Initialize("Resources/TD3_3102/2d/white2x2.png");
	miniMapPortalOutline_->SetTextureCamera(miniMapCamera_.get());
	miniMapPortalOutline_->SetColor({0.02f, 0.04f, 0.08f, 0.95f});
	Transform miniMapPortalOutlineTransform{};
	miniMapPortalOutlineTransform.scale = {miniMapRadius_ * 2.34f, miniMapRadius_ * 2.34f, 1.0f};
	miniMapPortalOutlineTransform.rotate = {0.0f, 0.0f, 0.0f};
	miniMapPortalOutlineTransform.translate = {-1.2f, -0.8f, 1.49f};
	miniMapPortalOutline_->SetTransform(miniMapPortalOutlineTransform);

	miniMapBackSprite_ = std::make_unique<Sprite>();
	miniMapBackSprite_->Initialize(miniMapBackTextureHandle_);
	miniMapBackSprite_->SetAnchorPoint({0.5f, 0.5f});
	miniMapBackSprite_->SetPosition(miniMapScreenCenter_);
	miniMapBackSprite_->SetScale({miniMapRadius_ * 2.0f, miniMapRadius_ * 2.0f});
	miniMapBackSprite_->SetColor({0.04f, 0.08f, 0.12f, 0.65f});

	markerSprite_ = std::make_unique<Sprite>();
	markerSprite_->Initialize(miniMapBackTextureHandle_);
	markerSprite_->SetAnchorPoint({0.5f, 0.5f});
	markerSprite_->SetScale({8.0f, 8.0f});

	playerSprite_ = std::make_unique<Sprite>();
	playerSprite_->Initialize(miniMapPlayerTextureHandle_);
	playerSprite_->SetAnchorPoint({0.5f, 0.5f});
	playerSprite_->SetScale({16.0f, 16.0f});
	playerSprite_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
}
void MiniMap::AddObject(std::string name, Object3d* object, Vector4 color, float markerSize) {
	if (!object) {
		entries_.erase(name);
		return;
	}
	entries_[name] = Entry{object, color, markerSize};
}

void MiniMap::SetPlayerTranslate(Vector3 translate) { playerTranslate_ = translate; }

void MiniMap::SetPlayerYaw(float yaw) { playerYaw_ = yaw; }

void MiniMap::SetStage(std::string stageName) {
	if (stageName_ == stageName) {
		return;
	}

	stageName_ = std::move(stageName);
	Reset();
}

void MiniMap::Reset() {
	entries_.clear();
	visibleMarkers_.clear();
}

void MiniMap::UpdateCamera() {
	if (!camera_) {
		return;
	}

	const Vector3 cameraPos = {
	    playerTranslate_.x + minimapAnchorOffset_.x,
	    playerTranslate_.y + minimapAnchorOffset_.y,
	    playerTranslate_.z + minimapAnchorOffset_.z,
	};
	camera_->SetTranslate(cameraPos);
	camera_->Update();

	miniMapCamera_->SetTranslate({playerTranslate_.x, playerTranslate_.y + miniMapContentCameraHeight_, playerTranslate_.z});
	miniMapCamera_->Update();
	const float ndcX = (miniMapScreenCenter_.x / (static_cast<float>(WinApp::kClientWidth) * 0.5f)) - 1.0f;
	const float ndcY = 1.0f - (miniMapScreenCenter_.y / (static_cast<float>(WinApp::kClientHeight) * 0.5f));
	const float portalDepth = 15.0f;
	const float halfFovY = camera_->GetFovY() * 0.5f;
	const float halfHeight = std::tan(halfFovY) * portalDepth;
	const float halfWidth = halfHeight * camera_->GetAspectRatio();

	const Matrix4x4& cameraWorld = camera_->GetWorldMatrix();
	const Vector3 right = {cameraWorld.m[0][0], cameraWorld.m[0][1], cameraWorld.m[0][2]};
	const Vector3 up = {cameraWorld.m[1][0], cameraWorld.m[1][1], cameraWorld.m[1][2]};
	const Vector3 forward = {cameraWorld.m[2][0], cameraWorld.m[2][1], cameraWorld.m[2][2]};
	const Vector3 cameraWorldPos = camera_->GetWorldTranslate();

	Transform miniMapPortalTransform{};
	miniMapPortalTransform.scale = {miniMapRadius_ * 0.14f, miniMapRadius_ * 0.14f, 1.0f};
	miniMapPortalTransform.rotate = camera_->GetRotate();
	miniMapPortalTransform.translate = {
	    cameraWorldPos.x + (forward.x * portalDepth) + (right.x * (ndcX * halfWidth)) + (up.x * (ndcY * halfHeight)),
	    cameraWorldPos.y + (forward.y * portalDepth) + (right.y * (ndcX * halfWidth)) + (up.y * (ndcY * halfHeight)),
	    cameraWorldPos.z + (forward.z * portalDepth) + (right.z * (ndcX * halfWidth)) + (up.z * (ndcY * halfHeight)),
	};
	miniMapPortal_->SetTransform(miniMapPortalTransform);
}

void MiniMap::UpdateVisibleMarkers() {
	visibleMarkers_.clear();
	for (const auto& [name, entry] : entries_) {
		(void)name;
		if (!entry.object) {
			continue;
		}

		const Vector3 worldPos = entry.object->GetTranslate();
		const Vector3 offset = {
		    worldPos.x - playerTranslate_.x,
		    0.0f,
		    worldPos.z - playerTranslate_.z,
		};
		if (LengthXZ(offset) > range_) {
			continue;
		}

		visibleMarkers_.push_back({
		    {playerTranslate_.x + offset.x, playerTranslate_.y + markerHeight_, playerTranslate_.z + offset.z},
		    entry.color,
		    entry.markerSize,
		});
	}
}

void MiniMap::Update() {
	UpdateCamera();
	UpdateVisibleMarkers();
}
void MiniMap::Draw() {
	if (!miniMapPortalOutline_ || !miniMapPortal_ || !markerSprite_) {
		return;
	}

	SpriteCommon::GetInstance()->DrawCommon();
	miniMapBackSprite_->Update();
	miniMapBackSprite_->Draw();

	Object3dCommon::GetInstance()->DrawCommon();
	miniMapPortal_->Update();
	miniMapPortal_->Draw();

	SpriteCommon::GetInstance()->DrawCommon();
	for (const auto& marker : visibleMarkers_) {
		const float dx = marker.position.x - playerTranslate_.x;
		const float dz = marker.position.z - playerTranslate_.z;
		const float distance = std::sqrt((dx * dx) + (dz * dz));
		if (distance > range_) {
			continue;
		}
		const float scale = miniMapRadius_ / range_;
		const float cosYaw = std::cos(-playerYaw_);
		const float sinYaw = std::sin(-playerYaw_);
		const float rotatedX = (dx * cosYaw) - (dz * sinYaw);
		const float rotatedZ = (dx * sinYaw) + (dz * cosYaw);
		const Vector2 markerPos = {
		    miniMapScreenCenter_.x + (rotatedX * scale),
		    miniMapScreenCenter_.y + (rotatedZ * scale),
		};

		const float offsetX = markerPos.x - miniMapScreenCenter_.x;
		const float offsetY = markerPos.y - miniMapScreenCenter_.y;
		const float radius = std::sqrt((offsetX * offsetX) + (offsetY * offsetY));
		if (radius > (miniMapRadius_ - 8.0f)) {
			continue;
		}

		markerSprite_->SetPosition(markerPos);
		markerSprite_->SetColor(marker.color);
		markerSprite_->SetScale({marker.markerSize, marker.markerSize});
		markerSprite_->Update();
		markerSprite_->Draw();
	}
	if (playerSprite_) {
		playerSprite_->SetPosition(miniMapScreenCenter_);
		playerSprite_->SetRotation(0.0f);
		playerSprite_->Update();
		playerSprite_->Draw();
	}
}