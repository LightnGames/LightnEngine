
//定数バッファ　メンバ変数みたいなもん
cbuffer ConstBuff : register(b0)
{
    matrix mtxProj;
    matrix mtxView;
    matrix mtxWorld;
    float4 diffuse;
    float3 cameraPos;
}

//頂点ごとのデータ
struct VS_INPUT
{
	float3 Pos    : POSITION;
	float2 Tex    : TEXCOORD0;
	float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

//ピクセルシェーダーのPosはfloat4ダヨ!!!!!!!!!!
struct PS_INPUT
{
    float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};

PS_INPUT VS ( VS_INPUT input )
{
	PS_INPUT output;

    //頂点座標

    output.PosH = mul(float4(input.Pos, 1), mtxView);
    output.PosH = mul(output.PosH, mtxProj);
    output.PosL = input.Pos;

	return output;
}