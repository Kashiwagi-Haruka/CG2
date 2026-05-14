#pragma once

#include "Audio.h"
#include "GameObject/YoshidaMath/CollisionManager/Collider.h"
#include "Object3d/Object3d.h"
#include <array>
#include <memory>
class PlayerCamera;
class Camera;
class CoffeeTrivia {
private:
	SoundData triviaVoice_;
	static std::array<std::u32string, 6> strings_;
	static bool isSendStartTriviaMessage_;
	static bool isCurrentVoiceFinished_;
	static size_t triviaNum_;
	std::unique_ptr<Object3d> triviaObj_;
	Vector3 velocity_ = {0.0f, 0.0f, 0.0f};
	Vector3 spawnForward_ = {0.0f, 0.0f, 1.0f};
	PlayerCamera* playerCamera_ = nullptr;
	AABB localAABB_ = {};
	bool isActive_ = false;
	bool isLanded_ = false;
	static bool isRayHit_;

public:
	CoffeeTrivia();
	~CoffeeTrivia();
	void Initialize();
	void Update();
	void Draw();
	void SetVol(float vol);
	void SetCamera(Camera* camera);
	void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; }
	void Spawn(const Vector3& origin, const Vector3& forward);
	static bool IsRayHit() { return isRayHit_; }
	static bool GetIsSendStartTriviaMessage() { return isSendStartTriviaMessage_; }
	static bool GetIsCurrentVoiceFinished() { return isCurrentVoiceFinished_; }
	static std::u32string& GetString() { return strings_[triviaNum_]; }
};