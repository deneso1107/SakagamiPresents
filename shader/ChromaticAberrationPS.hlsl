cbuffer DistortionBuffer : register(b0)
{
    float time;
    float strength;
    float padding1;
    float padding2;
}

// テクスチャとサンプラー
Texture2D tex : register(t0);
SamplerState linearSampler : register(s0);

// バーテックスシェーダからの入力
struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PS_IN input) : SV_TARGET
{
    
    float2 uv = input.uv;
    float2 center = float2(0.5f, 0.5f);
    float2 offset = uv - center;
    float distance = length(offset);
    
    // === アニメーションパターン ===
    
    // パターン1: 基本的なループ（0から最大強度へ）
   // float loopTime = sin(time * 2.0f) * 0.5f + 0.5f; // 0-1の範囲でループ
    
    // パターン2: より滑らかなループ（smoothstep使用）
    float cycle = sin(time * 1.5f) * 0.5f + 0.5f;
    float loopTime = smoothstep(0.0f, 1.0f, cycle);
    
    // パターン3: 脈動効果（ハートビートのような）
    // float pulse = abs(sin(time * 3.0f));
    // float loopTime = smoothstep(0.0f, 1.0f, pulse);
    
    // パターン4: より複雑な波形
    // float wave1 = sin(time * 2.0f) * 0.5f + 0.5f;
    // float wave2 = sin(time * 3.7f) * 0.3f + 0.3f;
    // float loopTime = (wave1 + wave2) * 0.5f;
    
    // パターン5: 段階的変化
    // float stepped = floor(sin(time * 1.0f) * 2.0f + 2.0f) / 4.0f;
    // float loopTime = smoothstep(0.0f, 1.0f, stepped);
    
    // === 色収差の強度計算 ===
    
    // 基本強度設定
    float baseStrength = 0.02f; // 最小強度（ほぼ見えない）
    float maxStrength = 0.15f; // 最大強度
    
    // strengthパラメータも考慮（外部から制御可能）
    float finalMaxStrength = maxStrength * strength;
    
    // 時間による補間
    float aberrationStrength = lerp(baseStrength, finalMaxStrength, loopTime);
    
    // 距離に基づく色収差計算
    float aberration = distance * distance * aberrationStrength;
    float2 direction = normalize(offset);
    
    // === 色分離サンプリング ===
    
    float3 color = float3(0.0, 0.0, 0.0);
    
    // 赤チャンネル - より強く外側にシフト
    float2 redUV = uv + direction * aberration * 1.5f;
    color.r = tex.Sample(linearSampler, redUV).r;
    
    // 緑チャンネル - 複数サンプルの平均化で品質向上
    float2 greenUV1 = uv + direction * aberration * 0.2f;
    float2 greenUV2 = uv + direction * aberration * -0.2f;
    color.g = (tex.Sample(linearSampler, greenUV1).g +
               tex.Sample(linearSampler, greenUV2).g) * 0.5f;
    
    // 青チャンネル - より強く内側にシフト
    float2 blueUV = uv + direction * aberration * -1.5f;
    color.b = tex.Sample(linearSampler, blueUV).b;
    
    // アルファチャンネル
    float alpha = tex.Sample(linearSampler, uv).a;
    
    return float4(color, alpha);
    //float2 uv = input.uv;
    //float2 center = float2(0.5f, 0.5f);
    //float2 offset = uv - center;
    //float distance = length(offset);
    
    //// より現実的な非線形色収差
    //float aberrationStrength = 0.1; // 色収差の強度を調整
    //float aberration = distance * distance * aberrationStrength;
    //float2 direction = normalize(offset);
    
    //// より細かい色分離とサンプリング
    //float3 color = float3(0.0, 0.0, 0.0);
    
    //// 赤チャンネル - より強く外側にシフト
    //float2 redUV = uv + direction * aberration * 1.5;
    //color.r = tex.Sample(linearSampler, redUV).r;
    
    //// 緑チャンネル - 複数サンプルの平均化で品質向上
    //float2 greenUV1 = uv + direction * aberration * 0.2;
    //float2 greenUV2 = uv + direction * aberration * -0.2;
    //color.g = (tex.Sample(linearSampler, greenUV1).g +
    //           tex.Sample(linearSampler, greenUV2).g) * 0.5;
    
    //// 青チャンネル - より強く内側にシフト
    //float2 blueUV = uv + direction * aberration * -1.5;
    //color.b = tex.Sample(linearSampler, blueUV).b;
    
    //// アルファチャンネル
    //float alpha = tex.Sample(linearSampler, uv).a;
    
    //return float4(color, alpha);
}