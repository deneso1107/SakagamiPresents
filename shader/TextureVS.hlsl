#include "common.hlsl"
PS_IN main(in VS_IN In)
{
    PS_IN Out;
	
    // ビルボードの頂点は既にワールド座標なので、ワールド変換をスキップ
    matrix vp;
    vp = mul(View, Projection);
    Out.Position = mul(In.Position, vp);
    Out.TexCoord = In.TexCoord;
    Out.Diffuse = In.Diffuse * Material.Diffuse;
	
    return Out;
}