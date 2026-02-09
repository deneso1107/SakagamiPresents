#pragma once
#include <xaudio2.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <map>
#include <vector>
#include <string>

#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

struct SoundData 
{
    BYTE* pAudioData;
    DWORD audioDataSize;
    WAVEFORMATEX waveFormat;
};
class SoundManager
{
private:
    static SoundManager* s_instance;

    IXAudio2* pXAudio2;
    IXAudio2MasteringVoice* pMasteringVoice;


    std::map<std::string, SoundData> soundDataMap;
    IXAudio2SourceVoice* pBGMVoice;
    std::vector<IXAudio2SourceVoice*> pSEVoices;

    std::map<std::string, IXAudio2SourceVoice*> activeSEVoices;  // Ä¶’†‚ÌSEŠÇ—

    SoundManager() : pXAudio2(nullptr), pMasteringVoice(nullptr), pBGMVoice(nullptr){}
public:
    bool Initialize();
    void Shutdown();
    bool LoadSound(const std::string& key, const wchar_t* filename);
    void PlayBGM(const std::string&, bool loop = true, float volume = 1.0f);
    void StopBGM();
    void PlaySE(const std::string&, float volume=1.0f);
    void Update();

    static void Init()
    {
        if (!s_instance) {
            s_instance = new SoundManager();
            s_instance->Initialize();
        }
    }

    static void Uninit()
    {
        if (s_instance)
        {
            s_instance->Shutdown();
            delete s_instance;
            s_instance = nullptr;
        }
    }

    static SoundManager& GetInstance()
    {
        return *s_instance;
    }

    void TitleSoundLoad();
    void GameSoundLoad();
    void ResultSoundLoad();
    void AllSoundLoad();
};

