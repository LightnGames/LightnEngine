cbuffer VSConstantBuffer : register(b0)
{
    matrix mtxProj;
    matrix mtxView;
    matrix mtxWorld;
	float3 cameraPos;
}

struct VS_INPUT
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

#include "../ScreenQuad.hlsl"

PS_INPUT_SCREEN VS(VS_INPUT input)
{
    PS_INPUT_SCREEN output;

    //í∏ì_ç¿ïW
    float4 worldPos = mul(float4(input.Pos, 1), mtxWorld);
	
	output.Pos = float4(input.Pos, 1);
    output.Tex = input.Tex;

    return output;
}