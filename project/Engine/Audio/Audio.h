#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <wrl.h>
#include <xaudio2.h>
struct ChunkHeader {
	char id[4];
	int32_t size;
};

struct RiffHeader {
	ChunkHeader chunk;
	char type[4];
};
struct FormatChunk {
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};
struct SoundData {
	WAVEFORMATEX wfex;
	std::vector<BYTE> buffer;
	float volume = 1.0f;
};

class Audio {

	static std::unique_ptr<Audio> instance;

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;

	IXAudio2MasteringVoice* masterVoice_;

	HRESULT result_;
	struct ActiveVoice {
		IXAudio2SourceVoice* voice;
		const BYTE* audioData;
		bool isLoop;
	};

	std::vector<ActiveVoice> activeVoices_;

	void StopVoicesForSound(const SoundData& soundData);
	void StopAllVoices();

public:
	void Update();
	static Audio* GetInstance();
	void Finalize();
	void InitializeIXAudio();
	SoundData SoundLoadFile(const char* filename);
	void SoundUnload(SoundData* soundData);
	void SoundPlayWave(const SoundData& sounddata, bool isLoop = false);
	void SetSoundVolume(SoundData* soundData, float volume);
	Microsoft::WRL::ComPtr<IXAudio2> GetIXAudio2() { return xAudio2_; };
};
