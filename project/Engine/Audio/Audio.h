#pragma once
#include "AudioMixer.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <wrl.h>
#include <xapofx.h>
#include <xaudio2.h>
#include <xaudio2fx.h>
struct ChunkHeader {
	// チャンク識別子("RIFF" や "fmt " など)
	char id[4];
	// チャンク本体サイズ
	int32_t size;
};

struct RiffHeader {
	// RIFF チャンク情報
	ChunkHeader chunk;
	// RIFF の種類("WAVE")
	char type[4];
};
struct FormatChunk {
	// fmt チャンク情報
	ChunkHeader chunk;
	// 波形フォーマット情報
	WAVEFORMATEX fmt;
};
struct SoundData {
	SoundData();
	SoundData(const SoundData& other);
	SoundData(SoundData&& other) noexcept;
	SoundData& operator=(const SoundData& other);
	SoundData& operator=(SoundData&& other) noexcept;
	~SoundData();

	static const std::vector<SoundData*>& GetInstances();
	const WAVEFORMATEX* GetWaveFormat() const;

	// 再生時に使う波形フォーマット
	WAVEFORMATEX wfex;
	// WAVEFORMATEXTENSIBLE を含む完全なフォーマット情報
	std::vector<BYTE> waveFormatBlob;
	// PCM データ本体
	std::vector<BYTE> buffer;
	// このサウンドの既定音量(0.0f～1.0f)
	float volume = 1.0f;
	// エディター表示用の名前
	std::string debugName;
	// このサウンド専用のエフェクトチェーン
	std::vector<AudioMixer::EffectSettings> effects;

private:
	void RegisterInstance();
	void UnregisterInstance();
};

class Audio {

public:
	using MixerEffectType = AudioMixer::EffectType;
	using MixerEffectSettings = AudioMixer::EffectSettings;

	struct EditorSoundEntry {
		SoundData* soundData = nullptr;
		std::string name;
		bool isPlaying = false;
		bool isLoop = false;
	};

private:
	static std::unique_ptr<Audio> instance;

	// XAudio2 本体
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;

	// 出力先となるマスターボイス
	IXAudio2MasteringVoice* masterVoice_ = nullptr;

	// API 呼び出し結果保持用
	HRESULT result_;
	bool comInitialized_ = false;

	struct ActiveVoice {
		enum class PlaybackSource {
			Scene,
			EditorPreview,
		};

		// 実際に再生中のソースボイス
		IXAudio2SourceVoice* voice;
		// どの SoundData を再生しているか識別するための先頭アドレス
		const BYTE* audioData;
		// ループ再生中かどうか
		bool isLoop;
		// 元音声の再生が終わったか(エフェクトテイル中かを判定するため)
		bool sourceEnded = false;
		// 元音声の総サンプルフレーム数
		UINT64 totalSamples = 0;
		// ボイスに適用しているエフェクトインスタンス保持
		std::vector<Microsoft::WRL::ComPtr<IUnknown>> effectInstances;
		// どこから再生された音か
		PlaybackSource playbackSource = PlaybackSource::Scene;
	};

	// 現在再生中のボイス一覧
	std::vector<ActiveVoice> activeVoices_;
	// AudioMixer 相当のミキサー
	AudioMixer mixer_;

	// 指定サウンドに紐づく再生中ボイスのみ停止
	void StopVoicesForSound(const SoundData& soundData);
	// 指定ボイスへエフェクトチェーンを再構築して適用
	bool ApplyEffectsToVoice(IXAudio2SourceVoice* voice, const std::vector<MixerEffectSettings>& effects, std::vector<Microsoft::WRL::ComPtr<IUnknown>>& outInstances);
	// 再生中ボイスを再作成して最新の設定を反映
	bool RecreateActiveVoice(ActiveVoice& active, const SoundData& soundData);
	// 全ボイスを停止
	void StopAllVoices();
	// 指定ソースの全ボイスを停止
	void StopVoicesBySource(ActiveVoice::PlaybackSource playbackSource);
	// 内部用の音声再生
	void SoundPlayWaveInternal(const SoundData& soundData, bool isLoop, ActiveVoice::PlaybackSource playbackSource);

public:
	// 非ループ音声の終了監視と後始末
	void Update();
	// シングルトン取得
	static Audio* GetInstance();
	// Audio システム破棄
	void Finalize();
	// XAudio2 / MediaFoundation 初期化
	void InitializeIXAudio();
	// 音声ファイル読み込み
	SoundData SoundLoadFile(const char* filename);
	// 音声データ解放
	void SoundUnload(SoundData* soundData);
	// 音声再生
	void SoundPlayWave(const SoundData& sounddata, bool isLoop = false);
	// 指定サウンドを停止して先頭から再生(重ね再生しない)
	void SoundPlayWaveFromStart(const SoundData& soundData, bool isLoop = false);
	// エディターのお試し再生専用: 指定サウンドを停止して先頭から再生
	void SoundPlayPreviewFromStart(const SoundData& soundData, bool isLoop = false);
	// 指定サウンドの再生を停止
	void StopSound(const SoundData& soundData);
	// シーン再生由来の音をすべて停止
	void StopAllSceneSounds();
	// エディターのお試し再生由来の音をすべて停止
	void StopAllPreviewSounds();
	// 音量設定
	void SetSoundVolume(SoundData* soundData, float volume);
	// エディター表示用のサウンド情報を取得
	std::vector<EditorSoundEntry> GetEditorSoundEntries() const;
	// 指定サウンドのエフェクトチェーンを取得
	std::vector<MixerEffectSettings> GetSoundEffects(const SoundData* soundData) const;
	// 指定サウンドのエフェクトチェーンを置き換え
	void SetSoundEffects(SoundData* soundData, const std::vector<MixerEffectSettings>& effects);
	// 指定サウンドにエフェクトを追加
	void AddSoundEffect(SoundData* soundData, const MixerEffectSettings& effect);
	// 指定サウンドのエフェクトを削除
	void RemoveSoundEffect(SoundData* soundData, size_t index);
	// 指定サウンドのエフェクトをクリア
	void ClearSoundEffects(SoundData* soundData);
	// 指定サウンドの Reverb 有効/無効を設定
	void SetReverb(SoundData* soundData, bool enabled);
	// 指定サウンドの Echo 有効/無効を設定
	void SetEcho(SoundData* soundData, bool enabled);
	// 指定サウンドの Equalizer 有効/無効を設定
	void SetEqualizer(SoundData* soundData, bool enabled);
	// 指定サウンドの Limiter 有効/無効を設定
	void SetLimiter(SoundData* soundData, bool enabled);
	// エディター表示/設定保存用のエフェクトチェーンを取得
	std::vector<MixerEffectSettings> GetMixerEffects() const;
	// AudioMixer エフェクトチェーンをまとめて置き換え
	void SetMixerEffects(const std::vector<MixerEffectSettings>& effects);
	// AudioMixer エフェクトを末尾に追加
	void AddMixerEffect(const MixerEffectSettings& effect);
	// AudioMixer エフェクトを削除
	void RemoveMixerEffect(size_t index);
	// AudioMixer エフェクトチェーンをクリア(=デフォルト: 何もなし)
	void ClearMixerEffects();
	// デバッグ/エディター表示用にエフェクト名を返す
	static const char* GetMixerEffectTypeName(MixerEffectType type);
	// 指定したサウンドが再生完了しているか
	bool IsSoundFinished(const SoundData& soundData) const;
	// XAudio2 へのアクセス用 getter
	Microsoft::WRL::ComPtr<IXAudio2> GetIXAudio2() { return xAudio2_; };
};
