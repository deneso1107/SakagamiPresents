struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

VS_OUTPUT main(uint vertexID : SV_VertexID)
{
    VS_OUTPUT output;
    
    // 頂点ID から フルスクリーン三角形を生成
    float2 texcoord = float2((vertexID << 1) & 2, vertexID & 2);
    output.texCoord = texcoord;
    output.position = float4(texcoord * float2(2, -2) + float2(-1, 1), 0, 1);
    
    return output;
}