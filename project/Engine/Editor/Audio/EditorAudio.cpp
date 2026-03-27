#include "EditorAudio.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

void EditorAudio::Finalize() {
	playModeInitializedAudioNames_.clear();
	ResetForSceneChange();
}

void EditorAudio::ResetForSceneChange() {
	playModeInitializedAudioNames_.clear();
	savedAudioVolumes_.clear();
	savedAudioLoopEnabled_.clear();
	savedAudioEffects_.clear();
}

void EditorAudio::OnPlayModeChanged(bool isPlaying, bool wasPlaying) {
	Audio* audio = Audio::GetInstance();
	if (isPlaying) {
		if (audio) {
			audio->StopAllPreviewSounds();
			audio->StopAllSceneSounds();
		}
		if (!wasPlaying) {
			playModeInitializedAudioNames_.clear();
		}
	} else if (wasPlaying) {
		if (audio) {
			audio->StopAllSceneSounds();
		}
		playModeInitializedAudioNames_.clear();
	}
}

void EditorAudio::SaveToJson(nlohmann::json& audioJson) const {
	audioJson["sounds"] = nlohmann::json::array();
	Audio* audio = Audio::GetInstance();
	if (!audio) {
		return;
	}

	for (const auto& entry : audio->GetEditorSoundEntries()) {
		if (!entry.soundData || entry.name.empty()) {
			continue;
		}
		const auto loopIt = savedAudioLoopEnabled_.find(entry.name);
		const bool loopEnabled = loopIt != savedAudioLoopEnabled_.end() ? loopIt->second : false;
		nlohmann::json soundJson = {
		    {"name",        entry.name             },
            {"volume",      entry.soundData->volume},
            {"loopEnabled", loopEnabled            },
            {"effects",     nlohmann::json::array()}
        };
		for (const auto& effect : audio->GetSoundEffects(entry.soundData)) {
			soundJson["effects"].push_back(SerializeAudioEffect(effect));
		}
		audioJson["sounds"].push_back(std::move(soundJson));
	}
}

void EditorAudio::LoadFromJson(const nlohmann::json& audioJson) {
	if (!audioJson.is_object()) {
		return;
	}
	if (audioJson.contains("sounds") && audioJson["sounds"].is_array()) {
		for (const auto& soundJson : audioJson["sounds"]) {
			if (!soundJson.is_object() || !soundJson.contains("name") || !soundJson["name"].is_string() || !soundJson.contains("volume") || !soundJson["volume"].is_number()) {
				continue;
			}
			const std::string name = soundJson["name"].get<std::string>();
			savedAudioVolumes_[name] = std::clamp(soundJson["volume"].get<float>(), 0.0f, 1.0f);
			if (soundJson.contains("loopEnabled") && soundJson["loopEnabled"].is_boolean()) {
				savedAudioLoopEnabled_[name] = soundJson["loopEnabled"].get<bool>();
			}
			if (soundJson.contains("effects") && soundJson["effects"].is_array()) {
				std::vector<Audio::MixerEffectSettings> effects;
				for (const auto& effectJson : soundJson["effects"]) {
					Audio::MixerEffectSettings effect{};
					if (DeserializeAudioEffect(effectJson, effect)) {
						effects.push_back(effect);
					}
				}
				if (!effects.empty()) {
					savedAudioEffects_[name] = std::move(effects);
				}
			}
		}
	}
}

bool EditorAudio::DrawEditor(bool isPlaying, bool& hasUnsavedChanges) {
#ifdef USE_IMGUI
	Audio* audio = Audio::GetInstance();
	if (!audio) {
		ImGui::TextUnformatted("Audio system unavailable");
		return false;
	}
	auto entries = audio->GetEditorSoundEntries();
	if (entries.empty()) {
		ImGui::TextUnformatted("No tracked sounds.");
		return false;
	}

	for (size_t i = 0; i < entries.size(); ++i) {
		auto& entry = entries[i];
		if (!entry.soundData) {
			continue;
		}
		const bool shouldApplySavedState = !isPlaying || !playModeInitializedAudioNames_.contains(entry.name);
		const auto savedIt = savedAudioVolumes_.find(entry.name);
		if (shouldApplySavedState && savedIt != savedAudioVolumes_.end()) {
			audio->SetSoundVolume(entry.soundData, savedIt->second);
		}
		bool loopEnabled = false;
		const auto loopIt = savedAudioLoopEnabled_.find(entry.name);
		if (loopIt != savedAudioLoopEnabled_.end()) {
			loopEnabled = loopIt->second;
		}
		const auto effectsIt = savedAudioEffects_.find(entry.name);
		if (shouldApplySavedState && effectsIt != savedAudioEffects_.end()) {
			audio->SetSoundEffects(entry.soundData, effectsIt->second);
			if (!isPlaying) {
				savedAudioEffects_.erase(effectsIt);
			}
		}
		if (isPlaying && shouldApplySavedState) {
			playModeInitializedAudioNames_.insert(entry.name);
		}

		if (ImGui::TreeNode((entry.name + "##audio_" + std::to_string(i)).c_str())) {
			float volume = entry.soundData->volume;
			if (ImGui::SliderFloat(("Volume##audio_volume_" + std::to_string(i)).c_str(), &volume, 0.0f, 1.0f)) {
				audio->SetSoundVolume(entry.soundData, volume);
				savedAudioVolumes_[entry.name] = volume;
				hasUnsavedChanges = true;
			}
			if (ImGui::Checkbox(("Loop Playback##audio_loop_enabled_" + std::to_string(i)).c_str(), &loopEnabled)) {
				savedAudioLoopEnabled_[entry.name] = loopEnabled;
				hasUnsavedChanges = true;
			}

			auto effects = audio->GetSoundEffects(entry.soundData);
			if (ImGui::TreeNode(("Effects##audio_effects_" + std::to_string(i)).c_str())) {
				auto findSoundEffect = [&](Audio::MixerEffectType type) {
					for (const auto& effect : effects) {
						if (effect.type == type) {
							return effect;
						}
					}
					Audio::MixerEffectSettings effect{};
					effect.type = type;
					effect.enabled = false;
					return effect;
				};

				auto reverb = findSoundEffect(Audio::MixerEffectType::Reverb);
				auto echo = findSoundEffect(Audio::MixerEffectType::Echo);
				auto equalizer = findSoundEffect(Audio::MixerEffectType::Equalizer);
				auto limiter = findSoundEffect(Audio::MixerEffectType::Limiter);

				bool changed = false;
				if (ImGui::Checkbox(("Use Reverb##audio_effect_reverb_" + std::to_string(i)).c_str(), &reverb.enabled)) {
					audio->SetReverb(entry.soundData, reverb.enabled);
					changed = true;
				}
				if (ImGui::SliderFloat(("Reverb WetDryMix##audio_reverb_wd_" + std::to_string(i)).c_str(), &reverb.reverb.WetDryMix, 0.0f, 100.0f)) {
					reverb.enabled = true;
					changed = true;
				}
				if (ImGui::Checkbox(("Use Echo##audio_effect_echo_" + std::to_string(i)).c_str(), &echo.enabled)) {
					audio->SetEcho(entry.soundData, echo.enabled);
					changed = true;
				}
				if (ImGui::SliderFloat(("Echo WetDryMix##audio_echo_wd_" + std::to_string(i)).c_str(), &echo.echo.WetDryMix, 0.0f, 100.0f)) {
					echo.enabled = true;
					changed = true;
				}
				if (ImGui::SliderFloat(("Echo Feedback##audio_echo_fb_" + std::to_string(i)).c_str(), &echo.echo.Feedback, 0.0f, 100.0f)) {
					echo.enabled = true;
					changed = true;
				}
				if (ImGui::SliderFloat(("Echo Delay##audio_echo_delay_" + std::to_string(i)).c_str(), &echo.echo.Delay, FXECHO_MIN_DELAY, FXECHO_MAX_DELAY)) {
					echo.enabled = true;
					changed = true;
				}
				if (ImGui::Checkbox(("Use Equalizer##audio_effect_eq_" + std::to_string(i)).c_str(), &equalizer.enabled)) {
					audio->SetEqualizer(entry.soundData, equalizer.enabled);
					changed = true;
				}
				if (ImGui::SliderFloat(("EQ Gain0##audio_eq_g0_" + std::to_string(i)).c_str(), &equalizer.equalizer.Gain0, FXEQ_MIN_GAIN, FXEQ_MAX_GAIN)) {
					equalizer.enabled = true;
					changed = true;
				}
				if (ImGui::SliderFloat(("EQ Gain1##audio_eq_g1_" + std::to_string(i)).c_str(), &equalizer.equalizer.Gain1, FXEQ_MIN_GAIN, FXEQ_MAX_GAIN)) {
					equalizer.enabled = true;
					changed = true;
				}
				if (ImGui::SliderFloat(("EQ Gain2##audio_eq_g2_" + std::to_string(i)).c_str(), &equalizer.equalizer.Gain2, FXEQ_MIN_GAIN, FXEQ_MAX_GAIN)) {
					equalizer.enabled = true;
					changed = true;
				}
				if (ImGui::SliderFloat(("EQ Gain3##audio_eq_g3_" + std::to_string(i)).c_str(), &equalizer.equalizer.Gain3, FXEQ_MIN_GAIN, FXEQ_MAX_GAIN)) {
					equalizer.enabled = true;
					changed = true;
				}
				if (ImGui::Checkbox(("Use Limiter##audio_effect_limiter_" + std::to_string(i)).c_str(), &limiter.enabled)) {
					audio->SetLimiter(entry.soundData, limiter.enabled);
					changed = true;
				}
				int release = static_cast<int>(limiter.limiter.Release);
				int loudness = static_cast<int>(limiter.limiter.Loudness);
				if (ImGui::SliderInt(("Limiter Release##audio_limiter_release_" + std::to_string(i)).c_str(), &release, FXMASTERINGLIMITER_MIN_RELEASE, FXMASTERINGLIMITER_MAX_RELEASE)) {
					limiter.limiter.Release = static_cast<UINT32>(release);
					limiter.enabled = true;
					changed = true;
				}
				if (ImGui::SliderInt(("Limiter Loudness##audio_limiter_loudness_" + std::to_string(i)).c_str(), &loudness, FXMASTERINGLIMITER_MIN_LOUDNESS, FXMASTERINGLIMITER_MAX_LOUDNESS)) {
					limiter.limiter.Loudness = static_cast<UINT32>(loudness);
					limiter.enabled = true;
					changed = true;
				}

				if (changed) {
					auto updatedEffects = std::vector<Audio::MixerEffectSettings>{reverb, echo, equalizer, limiter};
					audio->SetSoundEffects(entry.soundData, updatedEffects);
					savedAudioEffects_[entry.name] = std::move(updatedEffects);
					hasUnsavedChanges = true;
				}
				ImGui::TreePop();
			}

			if (entry.isPlaying) {
				ImGui::Text("Playing (%s)", entry.isLoop ? "Loop" : "One-shot");
			} else {
				ImGui::TextUnformatted("Stopped");
			}
			if (ImGui::Button(("Play from Start##audio_play_" + std::to_string(i)).c_str())) {
				audio->SoundPlayPreviewFromStart(*entry.soundData, loopEnabled);
			}
			ImGui::SameLine();
			if (ImGui::Button(("Stop##audio_stop_" + std::to_string(i)).c_str())) {
				audio->StopSound(*entry.soundData);
			}
			ImGui::TreePop();
		}
	}
	return true;
#else
	(void)isPlaying;
	(void)hasUnsavedChanges;
	return false;
#endif
}

nlohmann::json EditorAudio::SerializeAudioEffect(const Audio::MixerEffectSettings& effect) {
	nlohmann::json effectJson;
	effectJson["type"] = static_cast<int>(effect.type);
	effectJson["enabled"] = effect.enabled;
	effectJson["reverb"] = {
	    {"WetDryMix", effect.reverb.WetDryMix}
    };
	effectJson["echo"] = {
	    {"WetDryMix", effect.echo.WetDryMix},
        {"Feedback",  effect.echo.Feedback },
        {"Delay",     effect.echo.Delay    }
    };
	effectJson["equalizer"] = {
	    {"FrequencyCenter0", effect.equalizer.FrequencyCenter0},
        {"Gain0",            effect.equalizer.Gain0           },
        {"Bandwidth0",       effect.equalizer.Bandwidth0      },
	    {"FrequencyCenter1", effect.equalizer.FrequencyCenter1},
        {"Gain1",            effect.equalizer.Gain1           },
        {"Bandwidth1",       effect.equalizer.Bandwidth1      },
	    {"FrequencyCenter2", effect.equalizer.FrequencyCenter2},
        {"Gain2",            effect.equalizer.Gain2           },
        {"Bandwidth2",       effect.equalizer.Bandwidth2      },
	    {"FrequencyCenter3", effect.equalizer.FrequencyCenter3},
        {"Gain3",            effect.equalizer.Gain3           },
        {"Bandwidth3",       effect.equalizer.Bandwidth3      }
    };
	effectJson["limiter"] = {
	    {"Release",  effect.limiter.Release },
        {"Loudness", effect.limiter.Loudness}
    };
	return effectJson;
}

bool EditorAudio::DeserializeAudioEffect(const nlohmann::json& effectJson, Audio::MixerEffectSettings& effect) {
	if (!effectJson.is_object()) {
		return false;
	}
	if (effectJson.contains("type") && effectJson["type"].is_number_integer()) {
		const int type = effectJson["type"].get<int>();
		if (type >= 0 && type <= static_cast<int>(Audio::MixerEffectType::Limiter)) {
			effect.type = static_cast<Audio::MixerEffectType>(type);
		}
	}
	if (effectJson.contains("enabled") && effectJson["enabled"].is_boolean()) {
		effect.enabled = effectJson["enabled"].get<bool>();
	}
	if (effectJson.contains("reverb") && effectJson["reverb"].is_object()) {
		const auto& v = effectJson["reverb"];
		if (v.contains("WetDryMix") && v["WetDryMix"].is_number())
			effect.reverb.WetDryMix = v["WetDryMix"].get<float>();
	}
	if (effectJson.contains("echo") && effectJson["echo"].is_object()) {
		const auto& v = effectJson["echo"];
		if (v.contains("WetDryMix") && v["WetDryMix"].is_number())
			effect.echo.WetDryMix = v["WetDryMix"].get<float>();
		if (v.contains("Feedback") && v["Feedback"].is_number())
			effect.echo.Feedback = v["Feedback"].get<float>();
		if (v.contains("Delay") && v["Delay"].is_number())
			effect.echo.Delay = v["Delay"].get<float>();
	}
	if (effectJson.contains("equalizer") && effectJson["equalizer"].is_object()) {
		const auto& v = effectJson["equalizer"];
		if (v.contains("FrequencyCenter0") && v["FrequencyCenter0"].is_number())
			effect.equalizer.FrequencyCenter0 = v["FrequencyCenter0"].get<float>();
		if (v.contains("Gain0") && v["Gain0"].is_number())
			effect.equalizer.Gain0 = v["Gain0"].get<float>();
		if (v.contains("Bandwidth0") && v["Bandwidth0"].is_number())
			effect.equalizer.Bandwidth0 = v["Bandwidth0"].get<float>();
		if (v.contains("FrequencyCenter1") && v["FrequencyCenter1"].is_number())
			effect.equalizer.FrequencyCenter1 = v["FrequencyCenter1"].get<float>();
		if (v.contains("Gain1") && v["Gain1"].is_number())
			effect.equalizer.Gain1 = v["Gain1"].get<float>();
		if (v.contains("Bandwidth1") && v["Bandwidth1"].is_number())
			effect.equalizer.Bandwidth1 = v["Bandwidth1"].get<float>();
		if (v.contains("FrequencyCenter2") && v["FrequencyCenter2"].is_number())
			effect.equalizer.FrequencyCenter2 = v["FrequencyCenter2"].get<float>();
		if (v.contains("Gain2") && v["Gain2"].is_number())
			effect.equalizer.Gain2 = v["Gain2"].get<float>();
		if (v.contains("Bandwidth2") && v["Bandwidth2"].is_number())
			effect.equalizer.Bandwidth2 = v["Bandwidth2"].get<float>();
		if (v.contains("FrequencyCenter3") && v["FrequencyCenter3"].is_number())
			effect.equalizer.FrequencyCenter3 = v["FrequencyCenter3"].get<float>();
		if (v.contains("Gain3") && v["Gain3"].is_number())
			effect.equalizer.Gain3 = v["Gain3"].get<float>();
		if (v.contains("Bandwidth3") && v["Bandwidth3"].is_number())
			effect.equalizer.Bandwidth3 = v["Bandwidth3"].get<float>();
	}
	if (effectJson.contains("limiter") && effectJson["limiter"].is_object()) {
		const auto& v = effectJson["limiter"];
		if (v.contains("Release") && v["Release"].is_number_unsigned())
			effect.limiter.Release = v["Release"].get<UINT32>();
		if (v.contains("Loudness") && v["Loudness"].is_number_unsigned())
			effect.limiter.Loudness = v["Loudness"].get<UINT32>();
	}
	return true;
}