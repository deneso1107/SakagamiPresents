//マテリアル定数バッファを追加
cbuffer Material : register(b3)
{
    float4 Ambient;
    float4 Diffuse; // Diffuse.w = アルファ値
    float4 Specular;
    float4 Emission;
    float Shiness;
    bool TextureEnable;
    float2 Dummy;
};

Texture2D mainTexture : register(t0);
SamplerState mainSampler : register(s0);

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // テクスチャから色を取得
    float4 color = mainTexture.Sample(mainSampler, input.tex);
    
    // ★★★ マテリアルのアルファを適用 ★★★
    color.a *= Diffuse.a;
    
    // ★★★ マテリアルのDiffuse色も適用（オプション） ★★★
    // color.rgb *= Diffuse.rgb;  // 色の調整が必要な場合
    
    return color;
}