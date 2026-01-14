cbuffer TransitionBuffer : register(b0)
{
    float slideOffset; // スライド位置
    float imageScale; // スケール
    float imageYPosition; // Y位置（揺れ用）
    float padding;
    //float imageAlpha; // ★画像のアルファ値    
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
    PS_INPUT output;
    
    // 画像を適切なサイズにスケーリング
    // ロケット牛は画面の1/3くらいのサイズが良いかも
    float imageWidth = 0.6f * imageScale; // 画面幅の40%
    float imageHeight = 0.6f * imageScale; // 画面高さの30%
    
    // 頂点位置を調整
    float4 position = float4(input.pos.xy, input.pos.z, 1.0f);
    
    // スケーリング適用
    position.x *= imageWidth;
    position.y *= imageHeight;
    
    // Y位置の調整（ローディング中の揺れ）
    position.y += imageYPosition;
    
    // スライド位置の適用（右から左へ）
    position.x += slideOffset * 2.0f;
    
    output.pos = position;
    output.uv = input.uv;
    
    return output;
}