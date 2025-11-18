Texture2D texY : register(t0); // Y plane (R8)
Texture2D texUV : register(t1); // UV plane (R8G8)
SamplerState samLinear : register(s0);

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float2 uv = input.tex;

    // --- Y サンプリング ---
    float y = texY.Sample(samLinear, uv).r;

    // --- UV は半分の解像度 ---
    float2 uv2 = uv * 0.5f;
    float2 uvSample = texUV.Sample(samLinear, uv2).rg;

    // --- YUV → RGB 変換 ---
    float U = uvSample.x - 0.5;
    float V = uvSample.y - 0.5;

    float r = y + 1.402 * V;
    float g = y - 0.344136 * U - 0.714136 * V;
    float b = y + 1.772 * U;

    return float4(r, g, b, 1.0);
}