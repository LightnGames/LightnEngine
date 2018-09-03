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

struct PS_OUTPUT
{
    float4 albedo : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 rme : SV_TARGET2;
};