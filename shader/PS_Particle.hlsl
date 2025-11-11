Texture2D tex : register(t0);
SamplerState samp : register(s0);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR0;
};

float4 main(PS_IN input) : SV_TARGET
{
    float4 texColor = tex.Sample(samp, input.uv);
    return texColor * input.color;
}