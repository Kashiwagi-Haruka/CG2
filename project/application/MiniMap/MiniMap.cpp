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

MiniMap* MiniMap::GetInstance() {
	if (!instance_) {
		instance_ = std::make_unique<MiniMap>();
	}
	return instance_.get();
}

void MiniMap::Finalize() { instance_.reset(); }

void MiniMap::Initialize() {
	Reset();

	if (isInitialized_) {
		return;
	}

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

	miniMapBackTextureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/white2x2.png");
	miniMapPlayerTextureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/MiniMapPlayer.png");

	miniMapBackSprite_ = std::make_unique<Sprite>();
	miniMapBackSprite_->Initialize(miniMapBackTextureHandle_);
	miniMapBackSprite_->SetAnchorPoint({0.5f, 0.5f});
	miniMapBackSprite_->SetPosition(miniMapScreenCenter_);
	miniMapBackSprite_->SetScale({miniMapRadius_ * 2.0f, miniMapRadius_ * 2.0f});
	miniMapBackSprite_->SetColor({0.04f, 0.08f, 0.12f, 0.65f});

	playerSprite_ = std::make_unique<Sprite>();
	playerSprite_->Initialize(miniMapPlayerTextureHandle_);
	playerSprite_->SetAnchorPoint({0.5f, 0.5f});
	playerSprite_->SetScale({16.0f, 16.0f});
	playerSprite_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});

	isInitialized_ = true;
}
void MiniMap::AddObject(Object3d* object, Vector4 color, float markerSize) {
	if (!object) {
		return;
	}

	for (auto& entry : entries_) {
		if (entry.object == object) {
			entry = Entry{object, color, markerSize};
			return;
		}
	}

	entries_.push_back(Entry{object, color, markerSize});
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
}

void MiniMap::UpdateVisibleMarkers() {
	visibleMarkers_.clear();
	for (const auto& entry : entries_) {
		if (!entry.object) {
			continue;
		}

		const Matrix4x4& worldMatrix = entry.object->GetWorldMatrix();
		const Vector3 worldPos = {
		    worldMatrix.m[3][0],
		    worldMatrix.m[3][1],
		    worldMatrix.m[3][2],
		};
		const Vector3 offset = {
		    worldPos.x - playerTranslate_.x,
		    0.0f,
		    worldPos.z - playerTranslate_.z,
		};
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

	SpriteCommon::GetInstance()->DrawCommon();
	miniMapBackSprite_->Update();
	miniMapBackSprite_->Draw();

	size_t markerIndex = 0;
	for (const auto& marker : visibleMarkers_) {
		const float dx = marker.position.x - playerTranslate_.x;
		const float dz = marker.position.z - playerTranslate_.z;
		const float scale = miniMapRadius_ / range_;
		const float cosYaw = std::cos(playerYaw_);
		const float sinYaw = std::sin(playerYaw_);
		const float rotatedX = (dx * cosYaw) - (dz * sinYaw);
		const float rotatedZ = (dx * sinYaw) + (dz * cosYaw);
		const Vector2 markerPos = {
		    miniMapScreenCenter_.x + (rotatedX * scale),
		    miniMapScreenCenter_.y - (rotatedZ * scale),
		};

		const float offsetX = markerPos.x - miniMapScreenCenter_.x;
		const float offsetY = markerPos.y - miniMapScreenCenter_.y;
		const float radius = std::sqrt((offsetX * offsetX) + (offsetY * offsetY));
		Vector2 drawPos = markerPos;
		const float clampRadius = miniMapRadius_ - 8.0f;
		if (radius > clampRadius && radius > 0.0f) {
			const float clampScale = clampRadius / radius;
			drawPos.x = miniMapScreenCenter_.x + (offsetX * clampScale);
			drawPos.y = miniMapScreenCenter_.y + (offsetY * clampScale);
		}

		if (markerSprites_.size() <= markerIndex) {
			auto sprite = std::make_unique<Sprite>();
			sprite->Initialize(miniMapBackTextureHandle_);
			sprite->SetAnchorPoint({0.5f, 0.5f});
			sprite->SetScale({8.0f, 8.0f});
			markerSprites_.push_back(std::move(sprite));
		}

		Sprite* markerSprite = markerSprites_[markerIndex].get();
		markerSprite->SetPosition(drawPos);
		markerSprite->SetColor(marker.color);
		markerSprite->SetScale({marker.markerSize, marker.markerSize});
		markerSprite->SetRotation(-playerYaw_);
		markerSprite->Update();
		markerSprite->Draw();
		++markerIndex;
	}
	if (playerSprite_) {
		playerSprite_->SetPosition(miniMapScreenCenter_);
		playerSprite_->SetRotation(0.0f);
		playerSprite_->Update();
		playerSprite_->Draw();
	}
}