TextureCube cubeMap : register(t0);
Texture2D texDiffuse : register(t1);
Texture2D texNormal : register(t2);
Texture2D texRoughness : register(t3);
Texture2D texMetallic : register(t4);
SamplerState samLinear : register(s0);

#include "../Gbuffer.hlsl"

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
	float4 albedo:SV_TARGET0;
	float4 normal:SV_TARGET1;
	float4 rme:SV_TARGET2;
};

PS_OUTPUT PS ( PS_INPUT input ) : SV_Target
{
	PS_OUTPUT output;

    //テクスチャをサンプル
    float4 baseColor = texDiffuse.Sample(samLinear, input.Tex);
    float3 normal = texNormal.Sample(samLinear, input.Tex);
    float roughness = texRoughness.Sample(samLinear, input.Tex).r;
    float metallic = texMetallic.Sample(samLinear, input.Tex);

	//ノーマルマップをワールドノーマルに適用
    normal = (normal * 2.0f) - 1.0f;
    normal = (normal.x * input.Tangent) + (normal.y * input.Binormal) + (normal.z * input.Normal);
    normal = normalize(normal);
    //normal = input.Normal;

	output.albedo = baseColor;
    output.normal = float4(EncodeNormal(normal), 1);
	output.rme = float4(roughness,metallic,0,0);

	return output;
}