#pragma once
#include <d3d11.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>
#include <cstdint>
#include <vector>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

using Microsoft::WRL::ComPtr;

class VideoPlayer
{
private:
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IMFSourceReader> sourceReader;

    ComPtr<ID3D11Texture2D> videoTexture;
    ComPtr<ID3D11ShaderResourceView> textureSRV;

    UINT videoWidth = 0;
    UINT videoHeight = 0;

    LONGLONG videoDuration = 0;
    LONGLONG currentTime = 0;

    bool isPlaying = false;
    bool isLooping = false;

    bool isNV12 = false;
    std::vector<uint8_t> rgbBuffer;

public:
    VideoPlayer() {}
    ~VideoPlayer() { Shutdown(); }

    HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

    HRESULT LoadVideo(const wchar_t* filename);
    HRESULT CreateVideoTexture();

    HRESULT Update(float deltaTime);
    HRESULT UpdateTexture(IMFSample* sample);

    void NV12ToRGB32(const uint8_t* yPlane,
        const uint8_t* uvPlane,
        int width, int height,
        int yStride, int uvStride,
        uint8_t* outRGBA);

    void Play() { isPlaying = true; }
    void Pause() { isPlaying = false; }
    void Stop() { isPlaying = false; Seek(0); }

    HRESULT Seek(LONGLONG timeInHundredNanos);

    ID3D11ShaderResourceView* GetTextureSRV() const { return textureSRV.Get(); }

    UINT GetWidth() const { return videoWidth; }
    UINT GetHeight() const { return videoHeight; }

    float GetCurrentTimeSeconds() const { return currentTime / 10000000.0f; }
    float GetDurationSeconds()  const { return videoDuration / 10000000.0f; }

    bool IsValid() const { return textureSRV != nullptr; }

    void SetLooping(bool loop) { isLooping = loop; }

    void Shutdown() {
        textureSRV.Reset();
        videoTexture.Reset();
        sourceReader.Reset();
        MFShutdown();
    }
};
//class VideoPlayer
//{
//private:
//    ComPtr<ID3D11Device> device;
//    ComPtr<ID3D11DeviceContext> context;
//    ComPtr<IMFSourceReader> sourceReader;
//    ComPtr<ID3D11Texture2D> videoTexture;
//    ComPtr<ID3D11ShaderResourceView> textureSRV;
//
//    ComPtr<ID3D11Texture2D> nv12Texture;
//    ComPtr<ID3D11ShaderResourceView> srvY;
//    ComPtr<ID3D11ShaderResourceView> srvUV;
//
//    UINT videoWidth;
//    UINT videoHeight;
//    LONGLONG videoDuration;
//    LONGLONG currentTime;
//    bool isPlaying;
//    bool isLooping;
//
//    std::vector<uint8_t> rgbBuffer;
//    bool isNV12 = false;
//
//public:
//    VideoPlayer() : videoWidth(0), videoHeight(0), videoDuration(0),
//        currentTime(0), isPlaying(false), isLooping(false) {
//    }
//
//    ~VideoPlayer() {
//        Shutdown();
//    }
//    HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
//	HRESULT LoadVideo(const wchar_t* filename);
//    HRESULT CreateVideoTexture();
//    HRESULT Update(float deltaTime);
//    HRESULT UpdateTexture(IMFSample* sample);
//    void NV12ToRGB32(const uint8_t*, const uint8_t*, int, int, int, int, uint8_t*);
//    void Play() { isPlaying = true; }
//    void Pause() { isPlaying = false; }
//    void Stop() {
//        isPlaying = false;
//        Seek(0);
//    }
//    HRESULT Seek(LONGLONG timeInHundredNanos);
//    // ÉQÉbÉ^Å[
//    ID3D11ShaderResourceView* GetTextureSRV() const { return textureSRV.Get(); }
//    UINT GetWidth() const { return videoWidth; }
//    UINT GetHeight() const { return videoHeight; }
//    bool IsPlaying() const { return isPlaying; }
//    void SetLooping(bool loop) { isLooping = loop; }
//    bool IsValid() const {
//        return videoTexture != nullptr && textureSRV != nullptr;
//    }
//    // åªç›ÇÃçƒê∂éûä‘ÅiïbÅj
//    float GetCurrentTimeSeconds() const {
//        return currentTime / 10000000.0f; // 100ÉiÉmïbíPà Ç©ÇÁïbÇ…ïœä∑
//    }
//
//    // ìÆâÊÇÃí∑Ç≥ÅiïbÅj
//    float GetDurationSeconds() const {
//        return videoDuration / 10000000.0f;
//    }
//
//    // èIóπèàóù
//    void Shutdown() {
//        textureSRV.Reset();
//        videoTexture.Reset();
//        sourceReader.Reset();
//        MFShutdown();
//    }
//};
//
