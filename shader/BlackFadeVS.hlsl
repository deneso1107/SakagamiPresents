cbuffer TransitionBuffer : register(b0)
{
    float fadeAlpha; // 0.0 Å` 1.0
    float3 padding;
};

struct VS_INPUT
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT o;
    o.pos = float4(input.pos.xy, 0.0f, 1.0f);
    o.uv = input.uv;
    return o;
}