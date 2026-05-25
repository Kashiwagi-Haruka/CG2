#include "MiniMap.h"

#include "Camera.h"
#include "Object3d/Object3d.h"
#include "Object3d/Object3dCommon.h"
#include "Sprite/Sprite.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"

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

	markerObject_ = std::make_unique<Object3d>();
	markerObject_->Initialize();
	markerObject_->SetModel("debugBox");
	markerObject_->SetCamera(camera_.get());
	markerObject_->SetEnableLighting(false);
	markerObject_->SetScale({0.2f, 0.2f, 0.2f});



	miniMapBackTextureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/white2x2.png");
	miniMapFrameTextureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/ring.png");

	miniMapBackSprite_ = std::make_unique<Sprite>();
	miniMapBackSprite_->Initialize(miniMapBackTextureHandle_);
	miniMapBackSprite_->SetAnchorPoint({0.5f, 0.5f});
	miniMapBackSprite_->SetPosition(miniMapScreenCenter_);
	miniMapBackSprite_->SetScale({miniMapRadius_ * 2.0f, miniMapRadius_ * 2.0f});
	miniMapBackSprite_->SetColor({0.04f, 0.08f, 0.12f, 0.65f});

	miniMapFrameSprite_ = std::make_unique<Sprite>();
	miniMapFrameSprite_->Initialize(miniMapFrameTextureHandle_);
	miniMapFrameSprite_->SetAnchorPoint({0.5f, 0.5f});
	miniMapFrameSprite_->SetPosition(miniMapScreenCenter_);
	miniMapFrameSprite_->SetScale({miniMapRadius_ * 2.2f, miniMapRadius_ * 2.2f});

	markerSprite_ = std::make_unique<Sprite>();
	markerSprite_->Initialize(miniMapBackTextureHandle_);
	markerSprite_->SetAnchorPoint({0.5f, 0.5f});
	markerSprite_->SetScale({8.0f, 8.0f});
}

void MiniMap::AddObject(std::string name, Object3d* object, Vector4 color) {
	if (!object) {
		entries_.erase(name);
		return;
	}
	entries_[name] = Entry{object, color};
}

void MiniMap::SetPlayerTranslate(Vector3 translate) { playerTranslate_ = translate; }

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
		});
	}
}

void MiniMap::Update() {
	UpdateCamera();
	UpdateVisibleMarkers();
}

void MiniMap::Draw() {
	if (!miniMapBackSprite_ || !miniMapFrameSprite_ || !markerSprite_) {
		return;
	}

	SpriteCommon::GetInstance()->DrawCommon();
	miniMapBackSprite_->Update();
	miniMapBackSprite_->Draw();

	for (const auto& marker : visibleMarkers_) {
		const float dx = marker.position.x - playerTranslate_.x;
		const float dz = marker.position.z - playerTranslate_.z;
		const float distance = std::sqrt((dx * dx) + (dz * dz));
		if (distance > range_) {
			continue;
		}
		const float scale = miniMapRadius_ / range_;
		const Vector2 markerPos = {
		    miniMapScreenCenter_.x + (dx * scale),
		    miniMapScreenCenter_.y + (dz * scale),
		};

		const float offsetX = markerPos.x - miniMapScreenCenter_.x;
		const float offsetY = markerPos.y - miniMapScreenCenter_.y;
		const float radius = std::sqrt((offsetX * offsetX) + (offsetY * offsetY));
		if (radius > (miniMapRadius_ - 8.0f)) {
			continue;
		}

		markerSprite_->SetPosition(markerPos);
		markerSprite_->SetColor(marker.color);
		markerSprite_->Update();
		markerSprite_->Draw();
	}

	miniMapFrameSprite_->Update();
	miniMapFrameSprite_->Draw();
}