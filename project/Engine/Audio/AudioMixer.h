#pragma once

#include <vector>
#include <wrl.h>
#include <xapofx.h>
#include <xaudio2.h>
#include <xaudio2fx.h>

class AudioMixer {
public:
	enum class EffectType {
		Reverb,
		Echo,
		Equalizer,
		Limiter,
	};

	struct EffectSettings {
		EffectType type = EffectType::Reverb;
		bool enabled = true;
		XAUDIO2FX_REVERB_PARAMETERS reverb{};
		FXECHO_PARAMETERS echo{};
		FXEQ_PARAMETERS equalizer{};
		FXMASTERINGLIMITER_PARAMETERS limiter{};

		EffectSettings();
	};

	AudioMixer() = default;
	~AudioMixer() = default;

	void Initialize(IXAudio2* xAudio2, IXAudio2MasteringVoice* masterVoice);
	void Finalize();

	bool IsReady() const { return mixerVoice_ != nullptr; }
	const XAUDIO2_VOICE_SENDS* GetOutputVoiceSends() const;

	void SetEffects(const std::vector<EffectSettings>& effects);
	void AddEffect(const EffectSettings& effect);
	void RemoveEffect(size_t index);
	void ClearEffects();
	std::vector<EffectSettings> GetEffects() const;

	static const char* GetEffectTypeName(EffectType type);

private:
	void RebuildEffectChain();

	IXAudio2SubmixVoice* mixerVoice_ = nullptr;
	std::vector<EffectSettings> effects_;
	std::vector<Microsoft::WRL::ComPtr<IUnknown>> effectInstances_;

	XAUDIO2_SEND_DESCRIPTOR sendDescriptor_{};
	XAUDIO2_VOICE_SENDS sendList_{};
};