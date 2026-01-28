#include "VideoPlayer.h"
#include <algorithm>

HRESULT VideoPlayer::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    device = pDevice;
    context = pContext;

    HRESULT hr = MFStartup(MF_VERSION);
    return hr;
}


HRESULT VideoPlayer::LoadVideo(const wchar_t* filename)
{
    HRESULT hr;

    hr = MFCreateSourceReaderFromURL(filename, nullptr, &sourceReader);
    if (FAILED(hr)) return hr;

    // ---- ストリーム設定 ---------------------------------
    sourceReader->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, FALSE);
    sourceReader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);

    // ---- RGB32 を試す ------------------------------------
    ComPtr<IMFMediaType> mediaType;
    MFCreateMediaType(&mediaType);

    mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);

    hr = sourceReader->SetCurrentMediaType(
        MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, mediaType.Get());

    if (SUCCEEDED(hr)) {
        OutputDebugStringA("RGB32 で読み込み成功！\n");
        isNV12 = false;
    }
    else {
        OutputDebugStringA("RGB32 がサポートされない → NV12 へ\n");

        mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
        hr = sourceReader->SetCurrentMediaType(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, mediaType.Get());

        if (FAILED(hr)) return hr;
        OutputDebugStringA("NV12 で読み込み成功！\n");
        isNV12 = true;
    }

    // ---- 実際のフォーマットを取得 ------------------------
    ComPtr<IMFMediaType> outType;
    hr = sourceReader->GetCurrentMediaType(
        MF_SOURCE_READER_FIRST_VIDEO_STREAM, &outType);

    MFGetAttributeSize(outType.Get(), MF_MT_FRAME_SIZE, &videoWidth, &videoHeight);

    // ---- 長さ取得 ---------------------------------------
    PROPVARIANT var;
    PropVariantInit(&var);
    if (SUCCEEDED(sourceReader->GetPresentationAttribute(
        MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var)))
    {
        videoDuration = var.uhVal.QuadPart;
        PropVariantClear(&var);
    }

    // ---- GPU テクスチャ作成 -----------------------------
    CreateVideoTexture();

    rgbBuffer.resize(videoWidth * videoHeight * 4);

    currentTime = 0;

    return S_OK;
}



HRESULT VideoPlayer::CreateVideoTexture()
{
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = videoWidth;
    desc.Height = videoHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = device->CreateTexture2D(&desc, nullptr, &videoTexture);
    if (FAILED(hr)) return hr;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
    srvd.Format = desc.Format;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MostDetailedMip = 0;
    srvd.Texture2D.MipLevels = 1;

    hr = device->CreateShaderResourceView(videoTexture.Get(), &srvd, &textureSRV);
    return hr;
}



HRESULT VideoPlayer::Update(float deltaTime)
{
    if (!isPlaying || !sourceReader) return S_OK;

    ComPtr<IMFSample> sample;
    DWORD flags = 0;
    LONGLONG timestamp = 0;

    HRESULT hr = sourceReader->ReadSample(
        MF_SOURCE_READER_FIRST_VIDEO_STREAM,
        0, nullptr, &flags, &timestamp, &sample);

    if (FAILED(hr)) return hr;

    if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
        if (isLooping) Seek(0);
        else isPlaying = false;
        return S_OK;
    }

    if (sample) {
        UpdateTexture(sample.Get());
        currentTime = timestamp;
    }

    return S_OK;
}



HRESULT VideoPlayer::UpdateTexture(IMFSample* sample)
{
    if (!sample || !videoTexture || !context) return E_FAIL;

    ComPtr<IMFMediaBuffer> buffer;
    HRESULT hr = sample->ConvertToContiguousBuffer(&buffer);
    if (FAILED(hr)) return hr;

    // Try to get IMF2DBuffer to obtain pitch reliably
    ComPtr<IMF2DBuffer> buffer2D;
    BYTE* yPtr = nullptr;
    LONG yPitch = 0;
    BYTE* uvPtr = nullptr;
    LONG uvPitch = 0;
    BOOL used2D = FALSE;

    if (SUCCEEDED(buffer.As(&buffer2D))) {
        // Lock2D gives pointer to the start of the whole NV12 planes (Y then UV)
        hr = buffer2D->Lock2D(&yPtr, &yPitch);
        if (SUCCEEDED(hr)) {
            used2D = TRUE;
            // For NV12, UV plane starts at yPtr + yPitch * height
            uvPtr = yPtr + (yPitch * (LONG)videoHeight);
            // Usually uvPitch == yPitch, but we'll try to get stride for UV explicitly:
            // Some implementations use the same stride; if not available, we fallback to yPitch.
            uvPitch = yPitch; // safe default
        }
    }

    BYTE* rawData = nullptr;
    DWORD maxLen = 0, curLen = 0;
    if (!used2D) {
        // fallback to legacy IMFMediaBuffer::Lock
        hr = buffer->Lock(&rawData, &maxLen, &curLen);
        if (FAILED(hr)) {
            return hr;
        }
        // assume contiguous: Y at rawData, UV at rawData + width*height (may be padded)
        yPtr = rawData;
        yPitch = (LONG)videoWidth; // best-effort fallback
        uvPtr = rawData + (videoWidth * videoHeight);
        uvPitch = (LONG)videoWidth;
    }

    if (!isNV12) {
        // RGB32 copy as before (we assume sample contains full RGBA rows)
        D3D11_MAPPED_SUBRESOURCE map;
        hr = context->Map(videoTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
        if (SUCCEEDED(hr)) {
            // If we used rawData path, ensure curLen large enough (not checked here)
            for (UINT y = 0; y < videoHeight; ++y) {
                memcpy((BYTE*)map.pData + map.RowPitch * y,
                    yPtr + (size_t)yPitch * y,
                    videoWidth * 4);
            }
            context->Unmap(videoTexture.Get(), 0);
        }

        if (used2D) buffer2D->Unlock2D();
        else buffer->Unlock();
        return hr;
    }

    // NV12 path: convert to RGBA using correct strides
    rgbBuffer.resize(videoWidth * videoHeight * 4);

    NV12ToRGB32(yPtr, uvPtr, videoWidth, videoHeight, (int)yPitch, (int)uvPitch, rgbBuffer.data());

    // Write to GPU texture (B8G8R8A8)
    D3D11_MAPPED_SUBRESOURCE map;
    hr = context->Map(videoTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
    if (SUCCEEDED(hr)) {
        for (UINT row = 0; row < videoHeight; ++row) {
            memcpy((BYTE*)map.pData + map.RowPitch * row,
                rgbBuffer.data() + (size_t)videoWidth * 4 * row,
                videoWidth * 4);
        }
        context->Unmap(videoTexture.Get(), 0);
    }

    if (used2D) buffer2D->Unlock2D();
    else buffer->Unlock();

    return hr;
}




void VideoPlayer::NV12ToRGB32(
    const uint8_t* yPlane,
    const uint8_t* uvPlane,
    int width, int height,
    int yStride, int uvStride,
    uint8_t* outRGBA)
{
    for (int j = 0; j < height; ++j)
    {
        const uint8_t* yRow = yPlane + j * yStride;
        const uint8_t* uvRow = uvPlane + (j / 2) * uvStride;

        for (int i = 0; i < width; ++i)
        {
            int Y = (int)yRow[i];

            // NV12: UV are interleaved as U0 V0 U1 V1 ...
            int uvIndex = (i / 2) * 2;
            int U = (int)uvRow[uvIndex + 0];
            int V = (int)uvRow[uvIndex + 1];

            // If colors look wrong (green/blue), try swapping U and V:
            // int tmp = U; U = V; V = tmp;

            // convert to signed
            int C = Y - 16;
            int D = U - 128;
            int E = V - 128;

            int R = (298 * C + 409 * E + 128) >> 8;
            int G = (298 * C - 100 * D - 208 * E + 128) >> 8;
            int B = (298 * C + 516 * D + 128) >> 8;

            R = std::clamp(R, 0, 255);
            G = std::clamp(G, 0, 255);
            B = std::clamp(B, 0, 255);

            int idx = (j * width + i) * 4;
            outRGBA[idx + 0] = (uint8_t)B;
            outRGBA[idx + 1] = (uint8_t)G;
            outRGBA[idx + 2] = (uint8_t)R;
            outRGBA[idx + 3] = 255;
        }
    }
}



HRESULT VideoPlayer::Seek(LONGLONG time)
{
    if (!sourceReader) return E_FAIL;

    PROPVARIANT var;
    PropVariantInit(&var);
    var.vt = VT_I8;
    var.hVal.QuadPart = time;

    HRESULT hr = sourceReader->SetCurrentPosition(GUID_NULL, var);
    PropVariantClear(&var);

    currentTime = time;
    return hr;
}