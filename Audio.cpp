#include "Audio.h"
#include "fstream"
#include <assert.h>
#pragma comment(lib, "xAudio2.lib")

void Audio::InitializeIXAudio() {
	result_ = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result_)); // ここ追加
	result_ = xAudio2_->CreateMasteringVoice(&masterVoice_);
	assert(SUCCEEDED(result_)); // ここも追加
}


SoundData Audio::SoundLoadWave(const char* filename) {

	std::ifstream file;

	file.open(filename, std::ios_base::binary);
	assert(file.is_open());

	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	FormatChunk format = {};
	// ヘッダーだけ読み取ってから、サイズぶんだけ読み込む（安全）
	file.read((char*)&format.chunk, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}
	if (format.chunk.size > sizeof(WAVEFORMATEX)) {
		OutputDebugStringA("format.chunk.size が大きすぎる\n");
		assert(0);
	}
	

	file.read((char*)&format.fmt, format.chunk.size);
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));

	if (strncmp(data.id, "JUNK", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur);
		file.read((char*)&data, sizeof(data));
	}
	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	file.close();

	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.BufferSize = data.size;

	return soundData;
}
void Audio::SoundUnload(SoundData* soundData) {
	xAudio2_.Reset();
	delete[] soundData->pBuffer;
	soundData->pBuffer = 0;
	soundData->BufferSize = 0;
	soundData->wfex = {};
}

void Audio::SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData) {
	if (!xAudio2) {
		OutputDebugStringA("SoundPlayWave: xAudio2 is null!\n");
		return;
	}

	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result_ = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result_));

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.BufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	result_ = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(result_));

	result_ = pSourceVoice->Start();
	assert(SUCCEEDED(result_));
}
