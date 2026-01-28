#include "playerSwordTrail.h"
#include "Function.h"
#include <cmath>

namespace {
Vector3 Lerp(const Vector3& a, const Vector3& b, float t) { return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t}; }

float Length(const Vector3& v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

Matrix4x4 MakeSegmentMatrix(const Vector3& start, const Vector3& end, float width) {
	Vector3 direction = end - start;
	float length = Length(direction);
	if (length < 1e-4f) {
		return Function::MakeTranslateMatrix(start);
	}

	Vector3 xAxis = Function::Normalize(direction);
	Vector3 up = {0.0f, 1.0f, 0.0f};
	Vector3 zAxis = Function::Normalize(Function::Cross(xAxis, up));
	if (Length(zAxis) < 1e-4f) {
		up = {0.0f, 0.0f, 1.0f};
		zAxis = Function::Normalize(Function::Cross(xAxis, up));
	}
	if (Length(zAxis) < 1e-4f) {
		zAxis = {0.0f, 0.0f, 1.0f};
	}
	Vector3 yAxis = Function::Normalize(Function::Cross(zAxis, xAxis));

	Vector3 center = Lerp(start, end, 0.5f);

	Matrix4x4 world{};
	world.m[0][0] = xAxis.x * length;
	world.m[1][0] = xAxis.y * length;
	world.m[2][0] = xAxis.z * length;
	world.m[0][1] = yAxis.x * width;
	world.m[1][1] = yAxis.y * width;
	world.m[2][1] = yAxis.z * width;
	world.m[0][2] = zAxis.x;
	world.m[1][2] = zAxis.y;
	world.m[2][2] = zAxis.z;
	world.m[3][0] = center.x;
	world.m[3][1] = center.y;
	world.m[3][2] = center.z;
	world.m[3][3] = 1.0f;
	return world;
}
} // namespace

void PlayerSwordTrail::Initialize() {
	points_.clear();
	segments_.clear();
}

void PlayerSwordTrail::Clear() {
	points_.clear();
	segments_.clear();
}

void PlayerSwordTrail::EnsureSegmentCount(size_t count) {
	if (segments_.size() == count) {
		return;
	}
	if (segments_.size() < count) {
		while (segments_.size() < count) {
			auto primitive = std::make_unique<Primitive>();
			primitive->Initialize(Primitive::Band, "Resources/3d/Circle.png");
			primitive->SetEnableLighting(false);
			segments_.push_back(std::move(primitive));
		}
	} else {
		segments_.resize(count);
	}
}

void PlayerSwordTrail::UpdateSegments() {
	if (segments_.empty()) {
		return;
	}
	const size_t segmentCount = segments_.size();
	for (size_t i = 0; i < segmentCount; ++i) {
		const Vector3& start = points_[i];
		const Vector3& end = points_[i + 1];
		Primitive* segment = segments_[i].get();
		segment->SetCamera(camera_);
		segment->SetWorldMatrix(MakeSegmentMatrix(start, end, kSegmentWidth));
		float t = static_cast<float>(i + 1) / static_cast<float>(segmentCount);
		segment->SetColor({0.2f, 0.7f, 1.0f, 0.6f * t});
		segment->Update();
	}
}

void PlayerSwordTrail::Update(const Matrix4x4& swordWorldMatrix, bool isAttacking) {
	if (!isAttacking) {
		Clear();
		return;
	}

	Vector3 tipLocal = {0.0f, kTipLocalOffset, 0.0f};
	Vector3 tipWorld = Function::TransformVM(tipLocal, swordWorldMatrix);
	if (points_.empty()) {
		points_.push_back(tipWorld);
	} else {
		Vector3 delta = tipWorld - points_.back();
		if (Length(delta) >= kMinPointDistance) {
			points_.push_back(tipWorld);
		}
	}

	while (points_.size() > kMaxPoints) {
		points_.pop_front();
	}

	if (points_.size() < 2) {
		EnsureSegmentCount(0);
		return;
	}

	EnsureSegmentCount(points_.size() - 1);
	UpdateSegments();
}

void PlayerSwordTrail::Draw() {
	for (const auto& segment : segments_) {
		segment->Draw();
	}
}