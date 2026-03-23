#include "Audio.h"
#include "fstream"
#include <algorithm>
#include <array>
#include <assert.h>
#include <combaseapi.h>
#include <filesystem>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <string>
#include <utility>
#pragma comment(lib, "xAudio2.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")


namespace {
std::vector<SoundData*>& SoundDataRegistry() {
	static std::vector<SoundData*> instances;
	return instances;
}
UINT64 CalculateTotalSamples(const SoundData& soundData) {
	if (soundData.wfex.nBlockAlign == 0) {
		return 0;
	}
	return static_cast<UINT64>(soundData.buffer.size() / soundData.wfex.nBlockAlign);
}
std::vector<Audio::MixerEffectSettings> NormalizeEffects(const std::vector<Audio::MixerEffectSettings>& effects) {
	const std::array<Audio::MixerEffectType, 4> orderedTypes = {
	    Audio::MixerEffectType::Reverb,
	    Audio::MixerEffectType::Echo,
	    Audio::MixerEffectType::Equalizer,
	    Audio::MixerEffectType::Limiter,
	};

	std::vector<Audio::MixerEffectSettings> normalized;
	normalized.reserve(orderedTypes.size());
	for (Audio::MixerEffectType type : orderedTypes) {
		auto found = std::find_if(effects.begin(), effects.end(), [type](const Audio::MixerEffectSettings& effect) { return effect.type == type; });
		if (found != effects.end()) {
			normalized.push_back(*found);
			continue;
		}
		Audio::MixerEffectSettings effect{};
		effect.type = type;
		effect.enabled = false;
		normalized.push_back(effect);
	}
	return normalized;
}

void SetEffectEnabled(std::vector<Audio::MixerEffectSettings>& effects, Audio::MixerEffectType type, bool enabled) {
	auto normalized = NormalizeEffects(effects);
	auto target = std::find_if(normalized.begin(), normalized.end(), [type](const Audio::MixerEffectSettings& effect) { return effect.type == type; });
	if (target != normalized.end()) {
		target->enabled = enabled;
	}
	effects = std::move(normalized);
}

} // namespace
SoundData::SoundData() { RegisterInstance(); }

SoundData::SoundData(const SoundData& other) : wfex(other.wfex), waveFormatBlob(other.waveFormatBlob), buffer(other.buffer), volume(other.volume), debugName(other.debugName), effects(other.effects) {
	RegisterInstance();
}

SoundData::SoundData(SoundData&& other) noexcept
    : wfex(other.wfex), waveFormatBlob(std::move(other.waveFormatBlob)), buffer(std::move(other.buffer)), volume(other.volume), debugName(std::move(other.debugName)),
      effects(std::move(other.effects)) {
	RegisterInstance();
	other.wfex = {};
	other.waveFormatBlob.clear();
	other.volume = 1.0f;
	other.effects.clear();
}

SoundData& SoundData::operator=(const SoundData& other) {
	if (this == &other) {
		return *this;
	}
	wfex = other.wfex;
	waveFormatBlob = other.waveFormatBlob;
	buffer = other.buffer;
	volume = other.volume;
	debugName = other.debugName;
	effects = other.effects;
	return *this;
}

SoundData& SoundData::operator=(SoundData&& other) noexcept {
	if (this == &other) {
		return *this;
	}
	wfex = other.wfex;
	waveFormatBlob = std::move(other.waveFormatBlob);
	buffer = std::move(other.buffer);
	volume = other.volume;
	debugName = std::move(other.debugName);
	effects = std::move(other.effects);
	other.wfex = {};
	other.waveFormatBlob.clear();
	other.volume = 1.0f;
	return *this;
}

SoundData::~SoundData() { UnregisterInstance(); }

const std::vector<SoundData*>& SoundData::GetInstances() { return SoundDataRegistry(); }
const WAVEFORMATEX* SoundData::GetWaveFormat() const {
	if (!waveFormatBlob.empty()) {
		return reinterpret_cast<const WAVEFORMATEX*>(waveFormatBlob.data());
	}
	return &wfex;
}

void SoundData::RegisterInstance() { SoundDataRegistry().push_back(this); }

void SoundData::UnregisterInstance() {
	auto& instances = SoundDataRegistry();
	instances.erase(std::remove(instances.begin(), instances.end(), this), instances.end());
}


// Audio シングルトンインスタンス
std::unique_ptr<Audio> Audio::instance = nullptr;

// シングルトンインスタンスを返す
Audio* Audio::GetInstance() {
	if (instance == nullptr) {
		instance = std::make_unique<Audio>();
	}
	return instance.get();
}
// 再生中ボイスとオーディオAPIを終了する
void Audio::Finalize() {
	StopAllVoices();
	mixer_.Finalize();
	if (masterVoice_) {
		masterVoice_->DestroyVoice();
		masterVoice_ = nullptr;
	}
	xAudio2_.Reset();

	result_ = MFShutdown();
	assert(SUCCEEDED(result_));

	if (comInitialized_) {
		CoUninitialize();
		comInitialized_ = false;
	}

	instance = nullptr;
}

// Media Foundation / XAudio2 を初期化する
void Audio::InitializeIXAudio() {
	result_ = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (SUCCEEDED(result_)) {
		comInitialized_ = true;
	} else if (result_ == RPC_E_CHANGED_MODE) {
		// 既に別モデルで初期化済みでも COM は利用可能
		comInitialized_ = false;
		result_ = S_OK;
	} else {
		assert(SUCCEEDED(result_));
	}

	result_ = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(result_));
	result_ = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result_));
	result_ = xAudio2_->CreateMasteringVoice(&masterVoice_);
	assert(SUCCEEDED(result_));
	mixer_.Initialize(xAudio2_.Get(), masterVoice_);
}

// ワンショット再生が終わったボイスを回収する
void Audio::Update() {
	if (activeVoices_.empty()) {
		return;
	}

	for (auto& active : activeVoices_) {
		if (!active.voice) {
			continue;
		}
		if (active.isLoop) {
			continue;
		}

		XAUDIO2_VOICE_STATE state{};
		active.voice->GetState(&state);
		if (!active.sourceEnded && active.totalSamples > 0 && state.SamplesPlayed >= active.totalSamples) {
			active.sourceEnded = true;
		}
		if (state.BuffersQueued == 0) {
			active.voice->DestroyVoice();
			active.voice = nullptr;
		}
	}

	activeVoices_.erase(std::remove_if(activeVoices_.begin(), activeVoices_.end(), [](const ActiveVoice& active) { return active.voice == nullptr; }), activeVoices_.end());
}
// 音声ファイルを PCM として読み込んで SoundData を生成する
SoundData Audio::SoundLoadFile(const char* filename) {
	// フルパス → UTF-16 変換
	std::wstring filePathW;
	{
		int size = MultiByteToWideChar(CP_UTF8, 0, filename, -1, nullptr, 0);
		filePathW.resize(size);
		MultiByteToWideChar(CP_UTF8, 0, filename, -1, &filePathW[0], size);
	}

	HRESULT hr;
	Microsoft::WRL::ComPtr<IMFSourceReader> pReader;

	// SourceReader 作成
	hr = MFCreateSourceReaderFromURL(filePathW.c_str(), nullptr, &pReader);
	assert(SUCCEEDED(hr));

	// XAPOFX(Echo / EQ など) は FLOAT32 入力を前提とするため、
	// 読み込み時点で IEEE Float へデコードしておく
	Microsoft::WRL::ComPtr<IMFMediaType> pPCMType;
	hr = MFCreateMediaType(&pPCMType);
	assert(SUCCEEDED(hr));

	hr = pPCMType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	assert(SUCCEEDED(hr));

	hr = pPCMType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float);
	assert(SUCCEEDED(hr));

	hr = pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pPCMType.Get());
	assert(SUCCEEDED(hr));

	// 実際に適用されたメディアタイプ
	Microsoft::WRL::ComPtr<IMFMediaType> pOutType;
	hr = pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pOutType);
	assert(SUCCEEDED(hr));

	// WAVEFORMATEX / WAVEFORMATEXTENSIBLE を取り出す
	WAVEFORMATEX* waveFormat = nullptr;
	UINT32 waveFormatSize = 0;
	hr = MFCreateWaveFormatExFromMFMediaType(pOutType.Get(), &waveFormat, &waveFormatSize);
	assert(SUCCEEDED(hr));
	// SoundData へ格納
	SoundData soundData{};
	soundData.wfex = *waveFormat; // WAVEFORMATEX コピー
	soundData.waveFormatBlob.assign(reinterpret_cast<const BYTE*>(waveFormat), reinterpret_cast<const BYTE*>(waveFormat) + waveFormatSize);
	CoTaskMemFree(waveFormat);
	// 読み込んだ PCM データを格納するバッファ
	std::vector<BYTE> buffer;

	// 1フレームずつ読み込む
	while (true) {
		DWORD streamIndex = 0, flags = 0;
		LONGLONG llTimeStamp = 0;
		Microsoft::WRL::ComPtr<IMFSample> pSample;

		hr = pReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &streamIndex, &flags, &llTimeStamp, &pSample);

		assert(SUCCEEDED(hr));

		if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
			break;

		if (!pSample) {
			continue;
		} else {
			Microsoft::WRL::ComPtr<IMFMediaBuffer> pBuffer;
			hr = pSample->ConvertToContiguousBuffer(&pBuffer);
			assert(SUCCEEDED(hr));

			BYTE* pData = nullptr;
			DWORD maxLength = 0, curLength = 0;

			hr = pBuffer->Lock(&pData, &maxLength, &curLength);
			assert(SUCCEEDED(hr));

			soundData.buffer.insert(soundData.buffer.end(), pData, pData + curLength);

			pBuffer->Unlock();
		}
	}

	soundData.debugName = std::filesystem::path(filename).filename().string();

	return soundData;
}

// 指定サウンドに紐づく再生を止めてメモリを解放する
void Audio::SoundUnload(SoundData* soundData) {
	if (!soundData) {
		return;
	}
	StopVoicesForSound(*soundData);
	soundData->buffer.clear();
	soundData->wfex = {};
	soundData->waveFormatBlob.clear();
	soundData->effects.clear();
}

// サウンドを再生する(必要ならループ再生)
void Audio::SoundPlayWaveInternal(const SoundData& soundData, bool isLoop, ActiveVoice::PlaybackSource playbackSource) {
	if (!xAudio2_) {
		OutputDebugStringA("SoundPlayWave: xAudio2 is null!\n");
		return;
	}
	if (soundData.buffer.empty()) {
		OutputDebugStringA("SoundPlayWave: sound buffer is empty!\n");
		return;
	}

	IXAudio2SourceVoice* pSourceVoice = nullptr;
	const XAUDIO2_VOICE_SENDS* sendList = mixer_.GetOutputVoiceSends();
	const WAVEFORMATEX* waveFormat = soundData.GetWaveFormat();
	if (sendList) {
		result_ = xAudio2_->CreateSourceVoice(&pSourceVoice, waveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, sendList, nullptr);
	} else {
		result_ = xAudio2_->CreateSourceVoice(&pSourceVoice, waveFormat);
	}
	assert(SUCCEEDED(result_));
	if (FAILED(result_) || !pSourceVoice) {
		OutputDebugStringA("SoundPlayWave: CreateSourceVoice failed.\n");
		return;
	}

	pSourceVoice->SetVolume(soundData.volume);
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.buffer.data();
	buf.AudioBytes = static_cast<UINT32>(soundData.buffer.size());

	if (isLoop) {
		buf.LoopBegin = 0;
		buf.LoopLength = 0;
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
		buf.Flags = 0;
	} else {
		buf.Flags = XAUDIO2_END_OF_STREAM;
	}

	result_ = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(result_));
	ActiveVoice activeVoice{};
	activeVoice.voice = pSourceVoice;
	activeVoice.audioData = soundData.buffer.data();
	activeVoice.isLoop = isLoop;
	activeVoice.sourceEnded = false;
	activeVoice.totalSamples = isLoop ? 0 : CalculateTotalSamples(soundData);
	activeVoice.playbackSource = playbackSource;
	const bool effectApplied = ApplyEffectsToVoice(pSourceVoice, soundData.effects, activeVoice.effectInstances);
	assert(effectApplied);

	result_ = pSourceVoice->Start();
	assert(SUCCEEDED(result_));
	activeVoices_.push_back(std::move(activeVoice));
}

void Audio::SoundPlayWave(const SoundData& soundData, bool isLoop) { SoundPlayWaveInternal(soundData, isLoop, ActiveVoice::PlaybackSource::Scene); }

void Audio::SoundPlayWaveFromStart(const SoundData& soundData, bool isLoop) {
	StopVoicesForSound(soundData);
	SoundPlayWave(soundData, isLoop);
}

void Audio::SoundPlayPreviewFromStart(const SoundData& soundData, bool isLoop) {
	StopVoicesForSound(soundData);
	SoundPlayWaveInternal(soundData, isLoop, ActiveVoice::PlaybackSource::EditorPreview);
}
void Audio::StopSound(const SoundData& soundData) { StopVoicesForSound(soundData); }
// 特定サウンドを再生しているボイスだけを停止する
void Audio::StopVoicesForSound(const SoundData& soundData) {
	const BYTE* targetData = soundData.buffer.data();
	if (!targetData) {
		return;
	}

	for (auto& active : activeVoices_) {
		if (active.voice && active.audioData == targetData) {
			active.voice->Stop();
			active.voice->DestroyVoice();
			active.voice = nullptr;
		}
	}

	activeVoices_.erase(std::remove_if(activeVoices_.begin(), activeVoices_.end(), [](const ActiveVoice& active) { return active.voice == nullptr; }), activeVoices_.end());
}
// サウンド音量を更新し、再生中ボイスにも反映する
void Audio::SetSoundVolume(SoundData* soundData, float volume) {
	if (!soundData) {
		return;
	}
	soundData->volume = std::clamp(volume, 0.0f, 1.0f);
	const BYTE* targetData = soundData->buffer.data();
	if (!targetData) {
		return;
	}

	for (auto& active : activeVoices_) {
		if (active.voice && active.audioData == targetData) {
			active.voice->SetVolume(soundData->volume);
		}
	}
}

bool Audio::ApplyEffectsToVoice(IXAudio2SourceVoice* voice, const std::vector<MixerEffectSettings>& effects, std::vector<Microsoft::WRL::ComPtr<IUnknown>>& outInstances) {
	if (!voice) {
		return false;
	}

	XAUDIO2_VOICE_DETAILS voiceDetails{};
	voice->GetVoiceDetails(&voiceDetails);
	const UINT32 outputChannels = voiceDetails.InputChannels > 0 ? voiceDetails.InputChannels : 1;

	outInstances.clear();
	std::vector<XAUDIO2_EFFECT_DESCRIPTOR> descriptors;
	std::vector<const MixerEffectSettings*> appliedEffects;
	descriptors.reserve(effects.size());
	appliedEffects.reserve(effects.size());

	for (const auto& effect : effects) {
		if (!effect.enabled) {
			continue;
		}

		Microsoft::WRL::ComPtr<IUnknown> xapo;
		HRESULT hr = S_OK;
		switch (effect.type) {
		case MixerEffectType::Reverb:
			hr = XAudio2CreateReverb(&xapo, 0);
			break;
		case MixerEffectType::Echo:
			hr = CreateFX(__uuidof(FXEcho), reinterpret_cast<IUnknown**>(xapo.GetAddressOf()), nullptr, 0);
			break;
		case MixerEffectType::Equalizer:
			hr = CreateFX(__uuidof(FXEQ), reinterpret_cast<IUnknown**>(xapo.GetAddressOf()), nullptr, 0);
			break;
		case MixerEffectType::Limiter:
			hr = CreateFX(__uuidof(FXMasteringLimiter), reinterpret_cast<IUnknown**>(xapo.GetAddressOf()), nullptr, 0);
			break;
		default:
			hr = E_FAIL;
			break;
		}

		if (FAILED(hr) || !xapo) {
			continue;
		}

		outInstances.push_back(xapo);
		descriptors.push_back({xapo.Get(), TRUE, outputChannels});
		appliedEffects.push_back(&effect);
	}

	if (descriptors.empty()) {
		HRESULT hr = voice->SetEffectChain(nullptr);
		if (FAILED(hr)) {
			return false;
		}
		return true;
	}

	XAUDIO2_EFFECT_CHAIN chain{};
	chain.EffectCount = static_cast<UINT32>(descriptors.size());
	chain.pEffectDescriptors = descriptors.data();
	HRESULT hr = voice->SetEffectChain(&chain);
	if (FAILED(hr)) {
		return false;
	}

	for (UINT32 effectIndex = 0; effectIndex < appliedEffects.size(); ++effectIndex) {
		const auto& effect = *appliedEffects[effectIndex];
		switch (effect.type) {
		case MixerEffectType::Reverb:
			hr = voice->SetEffectParameters(effectIndex, &effect.reverb, sizeof(effect.reverb));
			break;
		case MixerEffectType::Echo:
			hr = voice->SetEffectParameters(effectIndex, &effect.echo, sizeof(effect.echo));
			break;
		case MixerEffectType::Equalizer:
			hr = voice->SetEffectParameters(effectIndex, &effect.equalizer, sizeof(effect.equalizer));
			break;
		case MixerEffectType::Limiter:
			hr = voice->SetEffectParameters(effectIndex, &effect.limiter, sizeof(effect.limiter));
			break;
		default:
			hr = E_FAIL;
			break;
		}
		if (FAILED(hr)) {
			return false;
		}
	}

	return true;
}

bool Audio::RecreateActiveVoice(ActiveVoice& active, const SoundData& soundData) {
	if (!active.voice) {
		return false;
	}

	IXAudio2SourceVoice* oldVoice = active.voice;
	IXAudio2SourceVoice* newVoice = nullptr;
	const XAUDIO2_VOICE_SENDS* sendList = mixer_.GetOutputVoiceSends();
	const WAVEFORMATEX* waveFormat = soundData.GetWaveFormat();
	if (sendList) {
		result_ = xAudio2_->CreateSourceVoice(&newVoice, waveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, sendList, nullptr);
	} else {
		result_ = xAudio2_->CreateSourceVoice(&newVoice, waveFormat);
	}
	if (FAILED(result_) || !newVoice) {
		return false;
	}

	newVoice->SetVolume(soundData.volume);

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.buffer.data();
	buf.AudioBytes = static_cast<UINT32>(soundData.buffer.size());
	if (active.isLoop) {
		buf.LoopBegin = 0;
		buf.LoopLength = 0;
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
		buf.Flags = 0;
	} else {
		buf.Flags = XAUDIO2_END_OF_STREAM;
	}

	result_ = newVoice->SubmitSourceBuffer(&buf);
	if (FAILED(result_)) {
		newVoice->DestroyVoice();
		return false;
	}

	if (!ApplyEffectsToVoice(newVoice, soundData.effects, active.effectInstances)) {
		newVoice->DestroyVoice();
		return false;
	}

	result_ = newVoice->Start();
	if (FAILED(result_)) {
		newVoice->DestroyVoice();
		return false;
	}

	oldVoice->Stop();
	oldVoice->DestroyVoice();

	active.voice = newVoice;
	active.audioData = soundData.buffer.data();
	return true;
}

std::vector<Audio::MixerEffectSettings> Audio::GetSoundEffects(const SoundData* soundData) const {
	if (!soundData) {
		return {};
	}
	return NormalizeEffects(soundData->effects);
}

void Audio::SetSoundEffects(SoundData* soundData, const std::vector<MixerEffectSettings>& effects) {
	if (!soundData) {
		return;
	}
	soundData->effects = NormalizeEffects(effects);

	const BYTE* targetData = soundData->buffer.data();
	if (!targetData) {
		return;
	}

	for (auto& active : activeVoices_) {
		if (!active.voice || active.audioData != targetData) {
			continue;
		}
		if (!RecreateActiveVoice(active, *soundData)) {
			continue;
		}
	}
}

void Audio::AddSoundEffect(SoundData* soundData, const MixerEffectSettings& effect) {
	if (!soundData) {
		return;
	}
	auto effects = NormalizeEffects(soundData->effects);
	auto target = std::find_if(effects.begin(), effects.end(), [&](const MixerEffectSettings& existing) { return existing.type == effect.type; });
	if (target != effects.end()) {
		*target = effect;
		target->enabled = true;
	}
	SetSoundEffects(soundData, effects);
}

void Audio::RemoveSoundEffect(SoundData* soundData, size_t index) {
	if (!soundData) {
		return;
	}
	auto effects = NormalizeEffects(soundData->effects);
	if (index >= effects.size()) {
		return;
	}
	effects[index].enabled = false;
	SetSoundEffects(soundData, effects);
}

void Audio::ClearSoundEffects(SoundData* soundData) {
	if (!soundData) {
		return;
	}
	auto effects = NormalizeEffects(soundData->effects);
	for (auto& effect : effects) {
		effect.enabled = false;
	}
	SetSoundEffects(soundData, effects);
}

void Audio::SetReverb(SoundData* soundData, bool enabled) {
	if (!soundData) {
		return;
	}
	auto effects = NormalizeEffects(soundData->effects);
	SetEffectEnabled(effects, MixerEffectType::Reverb, enabled);
	SetSoundEffects(soundData, effects);
}

void Audio::SetEcho(SoundData* soundData, bool enabled) {
	if (!soundData) {
		return;
	}
	auto effects = NormalizeEffects(soundData->effects);
	SetEffectEnabled(effects, MixerEffectType::Echo, enabled);
	SetSoundEffects(soundData, effects);
}

void Audio::SetEqualizer(SoundData* soundData, bool enabled) {
	if (!soundData) {
		return;
	}
	auto effects = NormalizeEffects(soundData->effects);
	SetEffectEnabled(effects, MixerEffectType::Equalizer, enabled);
	SetSoundEffects(soundData, effects);
}

void Audio::SetLimiter(SoundData* soundData, bool enabled) {
	if (!soundData) {
		return;
	}
	auto effects = NormalizeEffects(soundData->effects);
	SetEffectEnabled(effects, MixerEffectType::Limiter, enabled);
	SetSoundEffects(soundData, effects);
}

std::vector<Audio::EditorSoundEntry> Audio::GetEditorSoundEntries() const {
	std::vector<EditorSoundEntry> entries;
	const auto& instances = SoundData::GetInstances();
	entries.reserve(instances.size());
	for (SoundData* soundData : instances) {
		if (!soundData || soundData->buffer.empty()) {
			continue;
		}
		EditorSoundEntry entry{};
		entry.soundData = soundData;
		entry.name = soundData->debugName.empty() ? "(unnamed sound)" : soundData->debugName;
		for (const auto& active : activeVoices_) {
			if (!active.voice || active.audioData != soundData->buffer.data() || active.sourceEnded) {
				continue;
			}
			entry.isPlaying = true;
			entry.isLoop = entry.isLoop || active.isLoop;
		}
		entries.push_back(std::move(entry));
	}
	return entries;
}

std::vector<Audio::MixerEffectSettings> Audio::GetMixerEffects() const { return NormalizeEffects(mixer_.GetEffects()); }
void Audio::SetMixerEffects(const std::vector<MixerEffectSettings>& effects) { mixer_.SetEffects(NormalizeEffects(effects)); }

void Audio::AddMixerEffect(const MixerEffectSettings& effect) {
	auto effects = NormalizeEffects(mixer_.GetEffects());
	auto target = std::find_if(effects.begin(), effects.end(), [&](const MixerEffectSettings& existing) { return existing.type == effect.type; });
	if (target != effects.end()) {
		*target = effect;
		target->enabled = true;
	}
	mixer_.SetEffects(effects);
}

void Audio::RemoveMixerEffect(size_t index) {
	auto effects = NormalizeEffects(mixer_.GetEffects());
	if (index >= effects.size()) {
		return;
	}
	effects[index].enabled = false;
	mixer_.SetEffects(effects);
}

void Audio::ClearMixerEffects() {
	auto effects = NormalizeEffects(mixer_.GetEffects());
	for (auto& effect : effects) {
		effect.enabled = false;
	}
	mixer_.SetEffects(effects);
}

const char* Audio::GetMixerEffectTypeName(MixerEffectType type) { return AudioMixer::GetEffectTypeName(type); }

bool Audio::IsSoundFinished(const SoundData& soundData) const {
	const BYTE* targetData = soundData.buffer.data();
	if (!targetData) {
		return true;
	}
	for (const auto& active : activeVoices_) {
		if (active.voice && active.audioData == targetData && !active.sourceEnded) {
			return false;
		}
	}
	return true;
}

void Audio::StopAllVoices() {
	for (auto& active : activeVoices_) {
		if (!active.voice) {
			continue;
		}
		active.voice->Stop();
		active.voice->DestroyVoice();
		active.voice = nullptr;
	}
	activeVoices_.clear();
}
void Audio::StopVoicesBySource(ActiveVoice::PlaybackSource playbackSource) {
	for (auto& active : activeVoices_) {
		if (!active.voice || active.playbackSource != playbackSource) {
			continue;
		}
		active.voice->Stop();
		active.voice->DestroyVoice();
		active.voice = nullptr;
	}
	activeVoices_.erase(std::remove_if(activeVoices_.begin(), activeVoices_.end(), [](const ActiveVoice& active) { return active.voice == nullptr; }), activeVoices_.end());
}

void Audio::StopAllSceneSounds() { StopVoicesBySource(ActiveVoice::PlaybackSource::Scene); }

void Audio::StopAllPreviewSounds() { StopVoicesBySource(ActiveVoice::PlaybackSource::EditorPreview); }