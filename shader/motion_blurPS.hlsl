// モーションブラーと放射状のスピードライン、ショックウェーブを組み合わせたゲーム内演出で使用しているシェーダー
Texture2D sceneTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer PostProcessBuffer : register(b0)
{
    float blurStrength;
    float aberrationStrength;
    float centerX;
    float centerY;
    float time;
    float speedLineSpeed;
    float shockwaveIntensity; // 追加：ショックウェーブの強度
    float shockwaveProgress; // ←追加！
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float2 center = float2(centerX, centerY);
    float2 fromCenter = input.texCoord - center;
    float distance = length(fromCenter);
    float2 dir = normalize(fromCenter);
    
    // 中心に向かう方向
    float2 toCenter = -dir;
    // 中心からの距離を計算
    float edgeFactor = smoothstep(0.0, 0.7, distance);

    // === (1) 放射ブラー ===
    float4 blurColor = float4(0, 0, 0, 0);
    //ブラーサンプル数
    int blurSamples = 10;
    for (int i = 0; i < blurSamples; i++)
    {
        float t = i / float(blurSamples);
        // 中心に向かってサンプリング（距離を大きく）
        float offset = blurStrength * edgeFactor * t * 0.08;
        float2 sampleUV = input.texCoord + toCenter * offset;
        
        float chromaOffset = aberrationStrength * edgeFactor * t * 1.5;
        float2 chromaDir = toCenter * chromaOffset;
        
        float r = sceneTexture.Sample(samplerState, sampleUV + chromaDir).r;
        float g = sceneTexture.Sample(samplerState, sampleUV).g;
        float b = sceneTexture.Sample(samplerState, sampleUV - chromaDir).b;
        
        // サンプルを加算
        blurColor += float4(r, g, b, 1.0);
    }
    blurColor /= blurSamples;

    // === (2) スピードライン（放射状） ===
    float speedline = 0.0;
    if (distance > 0.05)//周りの線を短く
    {
        float angle = atan2(fromCenter.y, fromCenter.x);
        float lineCount = 320.0;
        float lineIndex = floor(angle / (6.28318 / lineCount)); 
        
        
        float seed = frac(sin(lineIndex * 12.9898) * 43758.5453); //ランダムシード生成
        float lineWidth = 0.01 + seed * 0.15; //太さをランダム化
        float distanceOffset = seed * 0.7; //長さをランダム化

        
        // 外方向スクロール（流速もランダム化）
        float flow = frac((distance + distanceOffset) * 1.6 + time * speedLineSpeed * (1.2 + seed * 0.8));
        float streak = smoothstep(0.0, 0.05, flow) * (1.0 - smoothstep(0.15, 0.35, flow));

        float linePattern = smoothstep(lineWidth, 0.0, abs(frac(angle * lineCount / 6.28318) - 0.5) * 2.0);
        if (seed < 0.3)
            linePattern = 0.0;

        float fade = smoothstep(0.15, 1.0, distance);
        speedline = linePattern * streak * fade * blurStrength * 3.0;
    }

    // === (3) ショックウェーブ ===
    float shockwave = 0.0;
    {
        float ringRadius = shockwaveProgress; // C++から渡された進行度
        float thickness = 0.05; // リングの厚さ
        float edge = smoothstep(ringRadius - thickness, ringRadius, distance) *
                 (1.0 - smoothstep(ringRadius, ringRadius + thickness, distance));
        shockwave = edge * shockwaveIntensity * (1.0 - distance);
    }

    // === (4) ホワイトアウト（中心の発光）　元々実装していたが、視認性を考慮し、コメントアウト ===
    //float whiteCore = pow(saturate(1.0 - distance * 8.0), 5.0) * (blurStrength + shockwaveIntensity * 2.0); //ここを変更すると強さが変わる

    // === 合成 ===
    float4 sceneColor = sceneTexture.Sample(samplerState, input.texCoord);
    float blurAmount = edgeFactor * blurStrength * 0.8;
    float3 finalColor = lerp(sceneColor.rgb, blurColor.rgb, blurAmount);

    // 青→白グラデーションでラインを発光
    float3 lineColor = lerp(float3(0.0, 0.4, 1.0), float3(1.0, 1.0, 1.0), distance);
    finalColor += lineColor * speedline * 1.5;

    // 青白いショックウェーブを加算
    finalColor += float3(0.3, 0.6, 1.0) * shockwave * 3.0;

    // 中心のホワイトアウト
    ///finalColor += float3(1.0, 1.0, 1.0) * whiteCore;

    return float4(saturate(finalColor), 1.0);
}