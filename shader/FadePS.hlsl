cbuffer FadeBuffer : register(b0)
{
    float fadeAlpha;
    float3 padding;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // 黒色にフェードアルファを適用
    return float4(0.0f, 0.0f, 0.0f, fadeAlpha);
}