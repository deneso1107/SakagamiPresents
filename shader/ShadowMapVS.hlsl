cbuffer WorldBuffer : register(b0)
{
    matrix World;
}

cbuffer ViewBuffer : register(b1)
{
    matrix View;
}

cbuffer ProjectionBuffer : register(b2)
{
    matrix Projection;
}

// register(b6)に変更（b5はBoneMatrixBufferと競合）
cbuffer LightMatrixBuffer : register(b6)
{
    matrix LightView;
    matrix LightProjection;
}

struct VS_IN
{
    float4 Position : POSITION0;
    float3 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
    int4 BoneIndex : BONEINDEX0;
    float4 BoneWeight : BONEWEIGHT0;
};

struct VS_OUT
{
    float4 Position : SV_POSITION;
    float Depth : TEXCOORD0;
};

VS_OUT main(VS_IN In)
{
    VS_OUT Out;
    
    // ワールド座標に変換
    float4 worldPos = mul(In.Position, World);
    
    // ライト空間に変換
    float4 lightViewPos = mul(worldPos, LightView);
    float4 lightProjPos = mul(lightViewPos, LightProjection);
    
    Out.Position = lightProjPos;
    Out.Depth = lightProjPos.z / lightProjPos.w;
    
    return Out;
}