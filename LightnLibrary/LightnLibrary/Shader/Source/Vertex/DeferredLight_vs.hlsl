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

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
	float3 Eye : POSITION0;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;

    //í∏ì_ç¿ïW
    float4 worldPos = mul(float4(input.Pos, 1), mtxWorld);
	
	output.Pos = float4(input.Pos, 1);
    output.Tex = input.Tex;
    output.Eye = normalize(worldPos.xyz - cameraPos);

    return output;
}