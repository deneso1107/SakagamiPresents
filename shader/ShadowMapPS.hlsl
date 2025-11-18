cbuffer LightMatrixBuffer : register(b6)
{
    matrix LightView;
    matrix LightProjection;
}

struct PS_IN
{
    float4 Position : SV_POSITION;
    float Depth : TEXCOORD0;
};

float4 main(PS_IN In) : SV_Target
{
    // 深度を可視化（グレースケール）
    float depth = In.Depth;
    return float4(depth, depth, depth, 1.0);
}