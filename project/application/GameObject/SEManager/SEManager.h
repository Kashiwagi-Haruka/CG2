#pragma once
#include "Audio.h"
#include <unordered_map>

class SEManager {
public:
	enum Data {
		DOOR_LOCK,
		DOOR_OPEN,
		FOOT_STEP,
		WARP,
		PORTAL_SPAWN,
		SHOT,
		PUSH_WATCH,
		NOISE,
		CHAIR,
		TYPE,
		VENDING_MAC,
		KEY,
		DESK,
		DAMAGE,
		CLOCK,
		CAN,
		BOX,
	};

private:
	static std::unordered_map<SEManager::Data, SoundData> SEs_;

public:
	static void Load();
	static void Initialize();
	static void Update();
	static void SetVol(float vol, const Data& data);
	static void SoundPlay(const Data& data, const bool loop = false);
	static void UnLoad();
	static void StopSound(const Data& data);
	static bool IsSoundFinished(const Data& data);
};