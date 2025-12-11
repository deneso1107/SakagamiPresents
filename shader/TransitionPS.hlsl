// TransitionPS.hlsl - ロケット牛用（黒フェード対応）
Texture2D transitionTexture : register(t0);
SamplerState transitionSampler : register(s0);

cbuffer TransitionBuffer : register(b0)
{
    float slideOffset;
    float imageScale;
    float imageYPosition;
    float imageAlpha; // 黒フェード用のアルファ値
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // テクスチャをサンプリング
    float4 color = transitionTexture.Sample(transitionSampler, input.uv);
    
    // ★画像全体のアルファ値を適用
    color.a *= imageAlpha;
    
    // アルファ値が低い部分は描画しない
    if (color.a < 0.01f)
        discard;
    
    return color;
}