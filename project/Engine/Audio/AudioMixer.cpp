#include "AudioMixer.h"

#include <assert.h>
#include <combaseapi.h>
#pragma comment(lib, "ole32.lib")

AudioMixer::EffectSettings::EffectSettings() {
	type = EffectType::Reverb;
	enabled = true;
	// 変化を感じやすいよう、既定は広い空間系のリバーブにする
	const XAUDIO2FX_REVERB_I3DL2_PARAMETERS preset = XAUDIO2FX_I3DL2_PRESET_AUDITORIUM;
	ReverbConvertI3DL2ToNative(&preset, &reverb);
	reverb.WetDryMix = 65.0f;

	// 既定値を強めて、ON 時に違いが出やすくする
	echo.WetDryMix = 60.0f;
	echo.Feedback = 65.0f;
	echo.Delay = 800.0f;
	equalizer.FrequencyCenter0 = 800.0f;
	equalizer.Gain0 = 4.5f;
	equalizer.Bandwidth0 = 18.0f;
	equalizer.FrequencyCenter1 = 2000.0f;
	equalizer.Gain1 = -5.0f;
	equalizer.Bandwidth1 = 18.0f;
	equalizer.FrequencyCenter2 = 8000.0f;
	equalizer.Gain2 = 6.0f;
	equalizer.Bandwidth2 = 18.0f;
	equalizer.FrequencyCenter3 = 12000.0f;
	equalizer.Gain3 = 3.0f;
	equalizer.Bandwidth3 = 18.0f;
	limiter.Release = FXMASTERINGLIMITER_DEFAULT_RELEASE;
	limiter.Loudness = FXMASTERINGLIMITER_DEFAULT_LOUDNESS;
}

void AudioMixer::Initialize(IXAudio2* xAudio2, IXAudio2MasteringVoice* masterVoice) {
	if (!xAudio2 || !masterVoice) {
		return;
	}
	if (mixerVoice_) {
		return;
	}

	XAUDIO2_VOICE_DETAILS masterVoiceDetails{};
	masterVoice->GetVoiceDetails(&masterVoiceDetails);

	HRESULT hr = xAudio2->CreateSubmixVoice(&mixerVoice_, masterVoiceDetails.InputChannels, masterVoiceDetails.InputSampleRate, 0, 0, nullptr, nullptr);
	assert(SUCCEEDED(hr));

	sendDescriptor_.Flags = 0;
	sendDescriptor_.pOutputVoice = mixerVoice_;
	sendList_.SendCount = 1;
	sendList_.pSends = &sendDescriptor_;

	RebuildEffectChain();
}

void AudioMixer::Finalize() {
	effectInstances_.clear();
	if (mixerVoice_) {
		mixerVoice_->DestroyVoice();
		mixerVoice_ = nullptr;
	}
	sendDescriptor_ = {};
	sendList_ = {};
}

const XAUDIO2_VOICE_SENDS* AudioMixer::GetOutputVoiceSends() const {
	if (!mixerVoice_) {
		return nullptr;
	}
	return &sendList_;
}

void AudioMixer::SetEffects(const std::vector<EffectSettings>& effects) {
	effects_ = effects;
	RebuildEffectChain();
}

void AudioMixer::AddEffect(const EffectSettings& effect) {
	effects_.push_back(effect);
	RebuildEffectChain();
}

void AudioMixer::RemoveEffect(size_t index) {
	if (index >= effects_.size()) {
		return;
	}
	effects_.erase(effects_.begin() + index);
	RebuildEffectChain();
}

void AudioMixer::ClearEffects() {
	effects_.clear();
	RebuildEffectChain();
}

std::vector<AudioMixer::EffectSettings> AudioMixer::GetEffects() const { return effects_; }

const char* AudioMixer::GetEffectTypeName(EffectType type) {
	switch (type) {
	case EffectType::Reverb:
		return "Reverb";
	case EffectType::Echo:
		return "Echo";
	case EffectType::Equalizer:
		return "EQ";
	case EffectType::Limiter:
		return "Limiter";
	default:
		return "Unknown";
	}
}

void AudioMixer::RebuildEffectChain() {
	if (!mixerVoice_) {
		return;
	}

	XAUDIO2_VOICE_DETAILS voiceDetails{};
	mixerVoice_->GetVoiceDetails(&voiceDetails);
	const UINT32 outputChannels = voiceDetails.InputChannels > 0 ? voiceDetails.InputChannels : 1;

	effectInstances_.clear();
	std::vector<XAUDIO2_EFFECT_DESCRIPTOR> descriptors;
	std::vector<const EffectSettings*> appliedEffects;
	descriptors.reserve(effects_.size());
	effectInstances_.reserve(effects_.size());
	appliedEffects.reserve(effects_.size());

	for (const auto& effect : effects_) {
		if (!effect.enabled) {
			continue;
		}

		Microsoft::WRL::ComPtr<IUnknown> xapo;
		HRESULT hr = S_OK;
		switch (effect.type) {
		case EffectType::Reverb:
			hr = XAudio2CreateReverb(&xapo, 0);
			break;
		case EffectType::Echo:
			hr = CreateFX(__uuidof(FXEcho), reinterpret_cast<IUnknown**>(xapo.GetAddressOf()), nullptr, 0);
			break;
		case EffectType::Equalizer:
			hr = CreateFX(__uuidof(FXEQ), reinterpret_cast<IUnknown**>(xapo.GetAddressOf()), nullptr, 0);
			break;
		case EffectType::Limiter:
			hr = CreateFX(__uuidof(FXMasteringLimiter), reinterpret_cast<IUnknown**>(xapo.GetAddressOf()), nullptr, 0);
			break;
		default:
			hr = E_FAIL;
			break;
		}

		if (FAILED(hr) || !xapo) {
			continue;
		}

		effectInstances_.push_back(xapo);
		descriptors.push_back({xapo.Get(), TRUE, outputChannels});
		appliedEffects.push_back(&effect);
	}

	if (descriptors.empty()) {
		HRESULT hr = mixerVoice_->SetEffectChain(nullptr);
		assert(SUCCEEDED(hr));
		return;
	}

	XAUDIO2_EFFECT_CHAIN chain{};
	chain.EffectCount = static_cast<UINT32>(descriptors.size());
	chain.pEffectDescriptors = descriptors.data();
	HRESULT hr = mixerVoice_->SetEffectChain(&chain);
	assert(SUCCEEDED(hr));

	for (UINT32 effectIndex = 0; effectIndex < appliedEffects.size(); ++effectIndex) {
		const auto& effect = *appliedEffects[effectIndex];
		switch (effect.type) {
		case EffectType::Reverb:
			hr = mixerVoice_->SetEffectParameters(effectIndex, &effect.reverb, sizeof(effect.reverb));
			break;
		case EffectType::Echo:
			hr = mixerVoice_->SetEffectParameters(effectIndex, &effect.echo, sizeof(effect.echo));
			break;
		case EffectType::Equalizer:
			hr = mixerVoice_->SetEffectParameters(effectIndex, &effect.equalizer, sizeof(effect.equalizer));
			break;
		case EffectType::Limiter:
			hr = mixerVoice_->SetEffectParameters(effectIndex, &effect.limiter, sizeof(effect.limiter));
			break;
		default:
			hr = E_FAIL;
			break;
		}
		assert(SUCCEEDED(hr));
	}
}