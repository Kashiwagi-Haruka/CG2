#pragma once
#include <xaudio2.h>
#include <cstdint>
#include <wrl.h>
#include <vector>
#include <memory>
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
	
};

class Audio {

	static std::unique_ptr<Audio> instance;

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;

	IXAudio2MasteringVoice* masterVoice_;

	HRESULT result_;

public:

	static Audio* GetInstance();
	void Finalize();
	void InitializeIXAudio();
	SoundData SoundLoadFile(const char* filename);
	void SoundUnload(SoundData* soundData);
	void SoundPlayWave(const SoundData& sounddata,bool isLoop=false);
	Microsoft::WRL::ComPtr<IXAudio2> GetIXAudio2() { return xAudio2_; };
}
;
