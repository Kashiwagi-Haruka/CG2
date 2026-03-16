#include "Audio.h"
#include "fstream"
#include <algorithm>
#include <assert.h>
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
} // namespace

SoundData::SoundData() { RegisterInstance(); }

SoundData::SoundData(const SoundData& other) : wfex(other.wfex), buffer(other.buffer), volume(other.volume), debugName(other.debugName) { RegisterInstance(); }

SoundData::SoundData(SoundData&& other) noexcept : wfex(other.wfex), buffer(std::move(other.buffer)), volume(other.volume), debugName(std::move(other.debugName)) {
	RegisterInstance();
	other.wfex = {};
	other.volume = 1.0f;
}

SoundData& SoundData::operator=(const SoundData& other) {
	if (this == &other) {
		return *this;
	}
	wfex = other.wfex;
	buffer = other.buffer;
	volume = other.volume;
	debugName = other.debugName;
	return *this;
}

SoundData& SoundData::operator=(SoundData&& other) noexcept {
	if (this == &other) {
		return *this;
	}
	wfex = other.wfex;
	buffer = std::move(other.buffer);
	volume = other.volume;
	debugName = std::move(other.debugName);
	other.wfex = {};
	other.volume = 1.0f;
	return *this;
}

SoundData::~SoundData() { UnregisterInstance(); }

const std::vector<SoundData*>& SoundData::GetInstances() { return SoundDataRegistry(); }

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

	instance = nullptr;
}

// Media Foundation / XAudio2 を初期化する
void Audio::InitializeIXAudio() {
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

	// PCM にデコードする設定
	Microsoft::WRL::ComPtr<IMFMediaType> pPCMType;
	hr = MFCreateMediaType(&pPCMType);
	assert(SUCCEEDED(hr));

	hr = pPCMType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	assert(SUCCEEDED(hr));

	hr = pPCMType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	assert(SUCCEEDED(hr));

	hr = pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pPCMType.Get());
	assert(SUCCEEDED(hr));

	// 実際に適用されたメディアタイプ
	Microsoft::WRL::ComPtr<IMFMediaType> pOutType;
	hr = pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pOutType);
	assert(SUCCEEDED(hr));

	// WAVEFORMATEX を取り出す
	WAVEFORMATEX* waveFormat = nullptr;
	hr = MFCreateWaveFormatExFromMFMediaType(pOutType.Get(), &waveFormat, nullptr);
	assert(SUCCEEDED(hr));
	// SoundData へ格納
	SoundData soundData{};
	soundData.wfex = *waveFormat; // WAVEFORMATEX コピー
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
}

// サウンドを再生する(必要ならループ再生)
void Audio::SoundPlayWave(const SoundData& soundData, bool isLoop) {
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
	if (sendList) {
		result_ = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex, 0, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, sendList, nullptr);
	} else {
		result_ = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	}
	assert(SUCCEEDED(result_));

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

	result_ = pSourceVoice->Start();
	assert(SUCCEEDED(result_));
	activeVoices_.push_back({pSourceVoice, soundData.buffer.data(), isLoop});
}

void Audio::SoundPlayWaveFromStart(const SoundData& soundData, bool isLoop) {
	StopVoicesForSound(soundData);
	SoundPlayWave(soundData, isLoop);
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
			if (!active.voice || active.audioData != soundData->buffer.data()) {
				continue;
			}
			entry.isPlaying = true;
			entry.isLoop = entry.isLoop || active.isLoop;
		}
		entries.push_back(std::move(entry));
	}
	return entries;
}

std::vector<Audio::MixerEffectSettings> Audio::GetMixerEffects() const { return mixer_.GetEffects(); }

void Audio::SetMixerEffects(const std::vector<MixerEffectSettings>& effects) { mixer_.SetEffects(effects); }

void Audio::AddMixerEffect(const MixerEffectSettings& effect) { mixer_.AddEffect(effect); }

void Audio::RemoveMixerEffect(size_t index) { mixer_.RemoveEffect(index); }

void Audio::ClearMixerEffects() { mixer_.ClearEffects(); }

const char* Audio::GetMixerEffectTypeName(MixerEffectType type) { return AudioMixer::GetEffectTypeName(type); }

// 指定したサウンドの非ループ再生がすべて完了しているかを返す
bool Audio::IsSoundFinished(const SoundData& soundData) const {
	const BYTE* targetData = soundData.buffer.data();
	if (!targetData) {
		return true;
	}

	for (const auto& active : activeVoices_) {
		if (!active.voice || active.audioData != targetData) {
			continue;
		}
		if (active.isLoop) {
			return false;
		}

		XAUDIO2_VOICE_STATE state{};
		active.voice->GetState(&state);
		if (state.BuffersQueued > 0) {
			return false;
		}
	}

	return true;
}

// すべての再生中ボイスを停止する
void Audio::StopAllVoices() {
	for (auto& active : activeVoices_) {
		if (active.voice) {
			active.voice->Stop();
			active.voice->DestroyVoice();
			active.voice = nullptr;
		}
	}
	activeVoices_.clear();
}