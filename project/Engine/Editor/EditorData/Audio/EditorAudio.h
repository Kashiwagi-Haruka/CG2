#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Engine/Audio/Audio.h"
#include "externals/nlohmann/json.hpp"

class EditorAudio {
public:
	void Finalize();
	void ResetForSceneChange();
	void OnPlayModeChanged(bool isPlaying, bool wasPlaying);
	void SaveToJson(nlohmann::json& audioJson) const;
	void LoadFromJson(const nlohmann::json& audioJson);
	bool DrawEditor(bool isPlaying, bool& hasUnsavedChanges);

private:
	static nlohmann::json SerializeAudioEffect(const Audio::MixerEffectSettings& effect);
	static bool DeserializeAudioEffect(const nlohmann::json& effectJson, Audio::MixerEffectSettings& effect);

	std::unordered_map<std::string, float> savedAudioVolumes_;
	std::unordered_map<std::string, bool> savedAudioLoopEnabled_;
	std::unordered_map<std::string, std::vector<Audio::MixerEffectSettings>> savedAudioEffects_;
	std::unordered_set<std::string> playModeInitializedAudioNames_;
};