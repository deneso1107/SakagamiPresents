Texture2D sceneTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer PostProcessBuffer : register(b0)
{
    float blurStrength; // ブラーの強さ (0.0 ~ 0.1)
    float aberrationStrength; // 色収差の強さ
    float2 center; // ブラーの中心 (通常は 0.5, 0.5)
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float2 dir = input.texCoord - center;
    float distance = length(dir);
    
    // 放射状ブラー
    float4 color = float4(0, 0, 0, 0);
    int samples = 10; // サンプル数（多いほど綺麗だが重い）
    
    for (int i = 0; i < samples; i++)
    {
        float scale = 1.0 - blurStrength * (i / float(samples));
        float2 sampleUV = center + dir * scale;
        
        // 色収差を加える
        float2 offset = dir * aberrationStrength * (1.0 + i * 0.1);
        
        float r = sceneTexture.Sample(samplerState, sampleUV + offset).r;
        float g = sceneTexture.Sample(samplerState, sampleUV).g;
        float b = sceneTexture.Sample(samplerState, sampleUV - offset).b;
        
        color += float4(r, g, b, 1.0);
    }
    
    color /= samples;
    return color;
}