cbuffer TransitionBuffer : register(b0)
{
    float slideOffset; // スライド位置
    float imageScale; // スケール
    float imageYPosition; // Y位置（揺れ用）
    float fadeAlpha;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};


float4 main(PS_INPUT input) : SV_TARGET
{
    return float4(0, 0, 0, fadeAlpha);
}