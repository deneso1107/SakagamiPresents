#include "common.hlsl"

Texture2D g_Texture : register(t0);
Texture2D g_ShadowMap : register(t1);
SamplerState g_SamplerState : register(s0);
SamplerState g_ShadowSampler : register(s1);

struct PS_IN_SHADOW
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Diffuse : COLOR0;
    float4 LightSpacePos : TEXCOORD1;
};

float4 main(in PS_IN_SHADOW In) : SV_Target
{
    float4 outDiffuse;
    
    // テクスチャサンプリング
    if (Material.TextureEnable)
    {
        outDiffuse = g_Texture.Sample(g_SamplerState, In.TexCoord);
        outDiffuse *= In.Diffuse;
    }
    else
    {
        outDiffuse = In.Diffuse;
    }
    
    // シャドウマップから影を計算
    float3 projCoords = In.LightSpacePos.xyz / In.LightSpacePos.w;
    
    // NDC座標[-1, 1]をテクスチャ座標[0, 1]に変換
    projCoords.x = projCoords.x * 0.5 + 0.5;
    projCoords.y = projCoords.y * -0.5 + 0.5;
    
    float shadow = 1.0;
    
    // テクスチャ座標の範囲内かチェック
    if (projCoords.x >= 0.0 && projCoords.x <= 1.0 &&
        projCoords.y >= 0.0 && projCoords.y <= 1.0 &&
        projCoords.z >= 0.0 && projCoords.z <= 1.0)
    {
        // シャドウマップから深度値を取得
        float closestDepth = g_ShadowMap.Sample(g_ShadowSampler, projCoords.xy).r;
        float currentDepth = projCoords.z;
        
        // シャドウアクネを防ぐバイアス
        float bias = 0.005;
        
        // 影判定
        if (currentDepth - bias > closestDepth)
        {
            shadow = 0.3; // 影の暗さ
        }
    }
    
    // 影を適用
    outDiffuse.rgb *= shadow;
    
    return outDiffuse;
}