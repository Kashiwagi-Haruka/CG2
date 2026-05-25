#include "MiniMap.h"

#include "Camera.h"
#include "Object3d/Object3d.h"
#include "Object3d/Object3dCommon.h"

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
	if (!markerObject_) {
		return;
	}
	Object3dCommon::GetInstance()->DrawCommonNoCullDepth();
	for (const auto& marker : visibleMarkers_) {
		markerObject_->SetTranslate(marker.position);
		markerObject_->SetColor(marker.color);
		markerObject_->Update();
		markerObject_->Draw();
	}
}