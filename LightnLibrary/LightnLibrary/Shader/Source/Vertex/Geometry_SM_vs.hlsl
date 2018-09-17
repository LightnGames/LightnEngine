#include "../DeferredGeometry.hlsl"

//定数バッファ　メンバ変数みたいなもん
cbuffer ConstBuff : register(b0)
{
    matrix mtxProj;
    matrix mtxView;
    matrix mtxWorld;
    float3 cameraPos;
}

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;

    //頂点座標
    float4 worldPos = mul(float4(input.Pos, 1), mtxWorld);
    output.Pos = worldPos;
    output.Pos = mul(output.Pos, mtxView);
    output.Pos = mul(output.Pos, mtxProj);
	
    //視点ベクトル
    output.Eye = normalize(worldPos.xyz - cameraPos);

    //テクスチャ
    output.Tex = input.Tex;
	
	//法線
    output.Normal = normalize(mul(input.Normal, (float3x3) mtxWorld));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) mtxWorld));
    output.Binormal = normalize(mul(input.Binormal, (float3x3) mtxWorld));

    return output;
}