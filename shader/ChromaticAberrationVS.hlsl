struct VS_INPUT
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    // フルスクリーンクアッド用の座標変換
    // 入力座標は既に -1～1 の範囲になっている前提
    output.pos = float4(input.pos.xy, 0.0, 1.0);
    output.uv = input.uv;
    
    return output;
}