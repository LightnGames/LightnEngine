
//定数バッファ　メンバ変数みたいなもん
cbuffer ConstBuff : register(b0)
{
    matrix mtxProj;
    matrix mtxView;
    matrix mtxWorld;
    float3 cameraPos;
}

//頂点ごとのデータ
struct VS_INPUT
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};


struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float3 Eye : POSITION0;
};

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