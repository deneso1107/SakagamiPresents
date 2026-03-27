#include "common.hlsl"

cbuffer HighlightBuffer : register(b6)
{
    float4 g_HighlightColor; // rgb=加算色, a=強度
}

PS_IN main(VS_IN In)
{
    PS_IN Out;
    
    // 通常のunlitTextureVSと同じ変換
    float4 worldPos = mul(In.Position, World);
    float4 viewPos = mul(worldPos, View);
    float4 projPos = mul(viewPos, Projection);
    Out.Position = projPos;
    Out.TexCoord = In.TexCoord;
    
    // ハイライト：Diffuseに明るさを加算
    Out.Diffuse = In.Diffuse + g_HighlightColor * g_HighlightColor.a;
    Out.Diffuse = clamp(Out.Diffuse, 0.0f, 2.0f); // 白飛び防止
    
    return Out;
}