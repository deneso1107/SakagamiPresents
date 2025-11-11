Texture2D mainTexture : register(t0);
SamplerState mainSampler : register(s0);

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    return mainTexture.Sample(mainSampler, input.tex);
}