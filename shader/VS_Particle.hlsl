//cbuffer ConstantBuffer : register(b0)
//{
//    matrix World;
//    matrix View;
//    matrix Projection;
//    float4 CameraPosition;
//    float4 LightDirection;
//    float4 MaterialAmbient;
//    float4 MaterialDiffuse;
//    float4 MaterialSpecular;
//    float4 MaterialEmission;
//    float MaterialShiness;
//    float TextureEnable;
//};

//struct VS_INPUT
//{
//    float3 position : POSITION;
//    float2 texcoord : TEXCOORD0;
    
//    // インスタンスデータ
//    float3 worldPosition : TEXCOORD1;
//    float size : TEXCOORD2;
//    float4 color : COLOR;
//};

//struct PS_INPUT
//{
//    float4 position : SV_POSITION;
//    float2 texcoord : TEXCOORD0;
//    float4 color : COLOR;
//};

//PS_INPUT main(VS_INPUT input)
//{
//    PS_INPUT output;
    
//    // ビルボード行列を作成
//    matrix invView = transpose(View);
//    invView._41_42_43_44 = float4(0, 0, 0, 1);
    
//    // ローカル座標にサイズを適用
//    float3 localPos = input.position * input.size;
    
//    // ビルボード回転を適用
//    float3 billboardPos = mul(float4(localPos, 0), invView).xyz;
    
//    // ワールド位置に配置
//    float3 worldPos = input.worldPosition + billboardPos;
    
//    // ビュー・プロジェクション変換
//    float4 viewPos = mul(float4(worldPos, 1.0f), View);
//    output.position = mul(viewPos, Projection);
    
//    output.texcoord = input.texcoord;
//    output.color = input.color;
    
//    return output;
//}

cbuffer WorldCB : register(b0)
{
    float4x4 world;
};
cbuffer ViewCB : register(b1)
{
    float4x4 view;
};
cbuffer ProjCB : register(b2)
{
    float4x4 proj;
};

struct VS_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;

    float4 i_world0 : WORLD0;
    float4 i_world1 : WORLD1;
    float4 i_world2 : WORLD2;
    float4 i_world3 : WORLD3;
    float4 i_color : COLOR0;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR0;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    // インスタンスワールド行列
    float4x4 instWorld = float4x4(
        input.i_world0,
        input.i_world1,
        input.i_world2,
        input.i_world3
    );

    // 正しい掛け順
    float4x4 worldFinal = mul(world, instWorld);

    // view * proj の結合
    float4x4 viewProj = mul(view, proj);

    // 座標変換（正しい順序）
    float4 localPos = float4(input.pos, 1.0f);
    output.pos = mul(mul(localPos, worldFinal), viewProj);

    output.uv = input.uv;
    output.color = input.i_color;
    return output;
}

//View行列用
//cbuffer ViewProjCB : register(b0)
//{
//    float4x4 viewProj;
//};

//struct VS_IN
//{
//    float3 pos : POSITION;
//    float2 uv : TEXCOORD;
    
//    // インスタンスデータ（スロット1）
//    float4 i_world0 : WORLD0;
//    float4 i_world1 : WORLD1;
//    float4 i_world2 : WORLD2;
//    float4 i_world3 : WORLD3;
//    float4 i_color : COLOR0;
//};

//struct VS_OUT
//{
//    float4 pos : SV_POSITION;
//    float2 uv : TEXCOORD;
//    float4 color : COLOR0;
//};

//VS_OUT main(VS_IN input)
//{
//    VS_OUT output;

//    float4x4 world = float4x4(
//        input.i_world0,
//        input.i_world1,
//        input.i_world2,
//        input.i_world3
//    );

//    float4 localPos = float4(input.pos, 1.0f);
//    output.pos = mul(localPos, world);
//    output.pos = mul(output.pos, viewProj);

//    output.uv = input.uv;
//    output.color = input.i_color;
//    return output;
//}