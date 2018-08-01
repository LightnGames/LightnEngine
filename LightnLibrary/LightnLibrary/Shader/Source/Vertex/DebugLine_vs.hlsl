
#define MAX_LINE_ARRAY 255

//定数バッファ
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
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;

    //頂点座標
    float4 worldPos = mul(float4(input.Pos, 1), mtxWorld);
    output.Pos = worldPos;
    output.Pos = mul(output.Pos, mtxView);
    output.Pos = mul(output.Pos, mtxProj);

    return output;
}