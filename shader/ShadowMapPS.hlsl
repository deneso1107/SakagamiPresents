struct PS_IN
{
    float4 Position : SV_POSITION;
    float Depth : TEXCOORD0;
};

float4 main(PS_IN In) : SV_Target
{
    // 深度を可視化（近い=暗い、遠い=明るい）
    float depth = In.Depth;
    return float4(depth, depth, depth, 1.0);
}