#pragma once
#include <xaudio2.h>
#include <cstdint>
#include <wrl.h>
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
	BYTE* pBuffer;
	unsigned int BufferSize;
};

class Audio {

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;

	IXAudio2MasteringVoice* masterVoice_;

	HRESULT result_;

public:

	void InitializeIXAudio();
	SoundData SoundLoadWave(const char* filename);
	void SoundUnload(SoundData* soundData);
	void SoundPlayWave(IXAudio2* xAudio2, const SoundData& sounddata);
	Microsoft::WRL::ComPtr<IXAudio2> GetIXAudio2() { return xAudio2_; };
}
;
