Texture2D sceneTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer PostProcessBuffer : register(b0)
{
    float blurStrength; // ブラーの強さ (0.0 ~ 0.1)
    float aberrationStrength; // 色収差の強さ
    float centerX; // ブラーの中心
    float centerY;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float2 center = float2(centerX, centerY);
    float2 dir = input.texCoord - center;
    float distance = length(dir);
    
    float4 color = float4(0, 0, 0, 0);
    float4 glowColor = float4(0, 0, 0, 0);
    int samples = 12;
    
    for (int i = 0; i < samples; i++)
    {
        float t = i / float(samples);
        float scale = 1.0 - blurStrength * t;
        float2 sampleUV = center + dir * scale;
        
        // 色収差
        float2 offset = dir * aberrationStrength * (1.0 + t * 0.2);
        
        float r = sceneTexture.Sample(samplerState, sampleUV + offset).r;
        float g = sceneTexture.Sample(samplerState, sampleUV).g;
        float b = sceneTexture.Sample(samplerState, sampleUV - offset).b;
        
        float4 sample = float4(r, g, b, 1.0);
        color += sample;
        
        // 外側のサンプルを白色として加算（グロー効果）
        float glowWeight = pow(t, 1.5) * blurStrength * 5.0;
        glowColor += float4(1, 1, 1, 1) * glowWeight;
    }
    
    color /= samples;
    glowColor /= samples;
    
    // 元の色 + 白いグロー
    color.rgb += glowColor.rgb;
    color.rgb = saturate(color.rgb);
    
    return color;
}