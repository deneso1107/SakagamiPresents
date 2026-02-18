Texture2D sceneTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer PostProcessBuffer : register(b0)
{
    float aberrationStrength;
    float time; //現在は使用していないが、将来的に時間経過による変化を加えるために用意
    float2 padding;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET //カメラレンズの色収差
{
    float2 dir = input.texCoord - 0.5;
    float2 offset = dir * aberrationStrength;
    
    float r = sceneTexture.Sample(samplerState, input.texCoord + offset).r;
    float g = sceneTexture.Sample(samplerState, input.texCoord).g;
    float b = sceneTexture.Sample(samplerState, input.texCoord - offset).b;
    
    return float4(r, g, b, 1.0);
}