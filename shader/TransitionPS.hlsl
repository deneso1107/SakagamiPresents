Texture2D transitionTexture : register(t0);
SamplerState transitionSampler : register(s0);

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // テクスチャをサンプリング
    float4 color = transitionTexture.Sample(transitionSampler, input.uv);
    
    // PNG画像の透明部分を処理
    // アルファ値が低い部分は描画しない（背景を透過）
    if (color.a < 0.01f)
        discard;
    
    // そのまま色を返す（ロケット牛の画像をそのまま表示）
    return color;
}