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

cbuffer LightMatrixBuffer : register(b5)
{
    matrix LightView;
    matrix LightProjection;
}

struct VS_IN
{
    float4 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Diffuse : COLOR;
    float2 TexCoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 Position : SV_POSITION; // float4を明示的に使用
    float Depth : TEXCOORD0;
};

VS_OUT main(VS_IN In)
{
    VS_OUT Out;
    
    // ワールド座標に変換
    float4 worldPos = mul(In.Position, World);
    
    // ライト空間のビュー変換
    float4 lightViewPos = mul(worldPos, LightView);
    
    // ライト空間のプロジェクション変換
    Out.Position = mul(lightViewPos, LightProjection);
    
    // 明示的に4成分すべてを保証
    Out.Position.w = max(Out.Position.w, 0.0001); // wが0にならないようにする
    
    // 深度値を保存（デバッグ用）
    Out.Depth = Out.Position.z / Out.Position.w;
    
    return Out;
}