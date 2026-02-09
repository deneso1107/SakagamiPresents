#include "SoundManager.h"
SoundManager* SoundManager::s_instance = nullptr;
bool SoundManager::Initialize()
{
    HRESULT hr;

    // COM初期化
    hr=CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) return false;

    // Media Foundation初期化
    MFStartup(MF_VERSION);

    // XAudio2初期化
    hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) return false;

    // マスタリングボイス作成
    hr = pXAudio2->CreateMasteringVoice(&pMasteringVoice);
    if (FAILED(hr)) return false;

    pBGMVoice = nullptr;

    return true;
}
void SoundManager::Shutdown()
{
    // BGM停止
    if (pBGMVoice) {
        pBGMVoice->DestroyVoice();
        pBGMVoice = nullptr;
    }

    // SE停止
    for (auto voice : pSEVoices) {
        if (voice) voice->DestroyVoice();
    }
    pSEVoices.clear();

    // サウンドデータ解放
    for (auto& pair : soundDataMap) {
        delete[] pair.second.pAudioData;
    }
    soundDataMap.clear();

    if (pMasteringVoice) {
        pMasteringVoice->DestroyVoice();
        pMasteringVoice = nullptr;
    }

    if (pXAudio2) {
        pXAudio2->Release();
        pXAudio2 = nullptr;
    }

    MFShutdown();
    CoUninitialize();
}
//MP3ファイル読み込み
bool SoundManager::LoadSound(const std::string& key, const wchar_t* filename)
{
    HRESULT hr;
    IMFSourceReader* pReader = nullptr;

    // ソースリーダー作成
    hr = MFCreateSourceReaderFromURL(filename, nullptr, &pReader);
    if (FAILED(hr)) return false;

    // オーディオストリーム選択
    pReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
    pReader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);

    // PCMに変換するメディアタイプ設定
    IMFMediaType* pPCMAudio = nullptr;
    MFCreateMediaType(&pPCMAudio);
    pPCMAudio->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    pPCMAudio->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    pReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pPCMAudio);
    pPCMAudio->Release();

    // フォーマット取得
    IMFMediaType* pMediaType = nullptr;
    pReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pMediaType);

    WAVEFORMATEX* pWaveFormat = nullptr;
    UINT32 formatSize = 0;
    MFCreateWaveFormatExFromMFMediaType(pMediaType, &pWaveFormat, &formatSize);

    SoundData soundData;
    soundData.waveFormat = *pWaveFormat;
    CoTaskMemFree(pWaveFormat);
    pMediaType->Release();

    // オーディオデータ読み込み
    std::vector<BYTE> audioData;
    while (true) {
        IMFSample* pSample = nullptr;
        DWORD flags = 0;

        hr = pReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            0, nullptr, &flags, nullptr, &pSample);

        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;
        if (FAILED(hr) || !pSample) break;

        IMFMediaBuffer* pBuffer = nullptr;
        pSample->ConvertToContiguousBuffer(&pBuffer);

        BYTE* pAudioBuffer = nullptr;
        DWORD bufferLength = 0;
        pBuffer->Lock(&pAudioBuffer, nullptr, &bufferLength);

        audioData.insert(audioData.end(), pAudioBuffer, pAudioBuffer + bufferLength);

        pBuffer->Unlock();
        pBuffer->Release();
        pSample->Release();
    }

    pReader->Release();

    // データ保存
    soundData.audioDataSize = (DWORD)audioData.size();
    soundData.pAudioData = new BYTE[soundData.audioDataSize];
    memcpy(soundData.pAudioData, audioData.data(), soundData.audioDataSize);

    soundDataMap[key] = soundData;

    return true;
}

void SoundManager::PlayBGM(const std::string& key, bool loop, float volume)
{
    if (soundDataMap.find(key) == soundDataMap.end()) return;

    // 既存のBGM停止
    if (pBGMVoice) {
        pBGMVoice->Stop();
        pBGMVoice->DestroyVoice();
    }

    SoundData& data = soundDataMap[key];

    // ソースボイス作成
    pXAudio2->CreateSourceVoice(&pBGMVoice, &data.waveFormat);

    // バッファ設定
    XAUDIO2_BUFFER buffer = { 0 };
    buffer.AudioBytes = data.audioDataSize;
    buffer.pAudioData = data.pAudioData;
    buffer.Flags = XAUDIO2_END_OF_STREAM;

    if (loop) {
        buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    }

    pBGMVoice->SubmitSourceBuffer(&buffer);
    pBGMVoice->SetVolume(volume);
    pBGMVoice->Start();
}

void SoundManager::StopBGM() 
{
    if (pBGMVoice) {
        pBGMVoice->Stop();
        pBGMVoice->DestroyVoice();
        pBGMVoice = nullptr;
    }
}

void SoundManager::PlaySE(const std::string& key, float volume)
{
    if (soundDataMap.find(key) == soundDataMap.end()) return;

    SoundData& data = soundDataMap[key];

    IXAudio2SourceVoice* pSEVoice = nullptr;
    pXAudio2->CreateSourceVoice(&pSEVoice, &data.waveFormat);

    XAUDIO2_BUFFER buffer = { 0 };
    buffer.AudioBytes = data.audioDataSize;
    buffer.pAudioData = data.pAudioData;
    buffer.Flags = XAUDIO2_END_OF_STREAM;

    pSEVoice->SubmitSourceBuffer(&buffer);
    pSEVoice->SetVolume(volume);
    pSEVoice->Start();

    pSEVoices.push_back(pSEVoice);
}

// 再生終了したSEボイスをクリーンアップ
void SoundManager::Update() {
    auto it = pSEVoices.begin();
    while (it != pSEVoices.end()) {
        XAUDIO2_VOICE_STATE state;
        (*it)->GetState(&state);

        if (state.BuffersQueued == 0) {
            (*it)->DestroyVoice();
            it = pSEVoices.erase(it);
        }
        else {
            ++it;
        }
    }
}
//タイトルで使うやつ
void SoundManager::TitleSoundLoad()
{
    LoadSound("titlebgm", L"assets/sound/bgm/Title.mp3");
    LoadSound("Click", L"assets/sound/se/クリック音.mp3");
}
void SoundManager::GameSoundLoad()
{
    LoadSound("bgm1", L"assets/sound/通常BGM.mp3");
    LoadSound("jump", L"assets/sound/ゲーム内打撃音_3.mp3");
}
void SoundManager::ResultSoundLoad()
{
    LoadSound("bgm1", L"assets/sound/通常BGM.mp3");
    LoadSound("jump", L"assets/sound/ゲーム内打撃音_3.mp3");
}

void SoundManager::AllSoundLoad()
{
	//タイトルBGM、ゲームBGM、リザルトBGMをまとめて読み込む
    LoadSound("titlebgm", L"assets/sound/bgm/Title.mp3");
    LoadSound("GameSceneNormalbgm", L"assets/sound/bgm/通常時.mp3");
    LoadSound("GameSceneAccerationbgm", L"assets/sound/bgm/無題.mp4");
    LoadSound("GameSceneAccerationbgm2", L"assets/sound/bgm/加速時の歓声.mp3");
    LoadSound("GameSceneFinal", L"assets/sound/bgm/Goal直前.mp3");
    LoadSound("ResultNormal", L"assets/sound/bgm/Result.mp3");
    LoadSound("ResultHigh", L"assets/sound/bgm/Resultハイスコア.mp3");
	//SEまとめて読み込み
    LoadSound("GameSceneFirst", L"assets/sound/se/始まり.mp3");
    LoadSound("fly_away", L"assets/sound/se/敵飛ばした時.mp3");
    LoadSound("Accerationse", L"assets/sound/se/加速時.mp3");
	LoadSound("countdown", L"assets/sound/se/カウントダウン.mp3");
	LoadSound("countdownfinal", L"assets/sound/se/カウントダウンラスト.mp3");
	LoadSound("Goal", L"assets/sound/se/Goal時.mp3");
	LoadSound("Click", L"assets/sound/se/クリック音.mp3");
}