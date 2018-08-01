Texture2D RT0 : register(t0);
Texture2D RT1 : register(t1);
Texture2D RT2 : register(t2);
TextureCube cubeMap : register(t3);
Texture2D depthTex : register(t4);
SamplerState samLinear : register(s0);

#include "PhysicallyBasedRendering.hlsl"
#include "DeferredLight.hlsl"

cbuffer PointLightInput : register(b0){
	float4 lightPosition;
	float4 lightColor;
	float4 lightAttenuation;
	matrix inverseViewProjection;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Eye : POSITION0;
};

float4 PS ( PS_INPUT input ) : SV_Target
{
    float4 baseColor;
	float4 rtMainColor;
    float4 normal;
	float roughness;
	float metallic;

    baseColor = RT0.Sample(samLinear, input.Tex);
    normal = RT1.Sample(samLinear, input.Tex);
	float4 RT2Value = RT2.Sample(samLinear, input.Tex);
	roughness = RT2Value.r;
	metallic = RT2Value.g;

	//メタリックの値からテクスチャカラー
    float3 diffuseColor = lerp(baseColor.xyz, float3(0.04, 0.04, 0.04), metallic);
    float3 specularColor = lerp(float3(0.04, 0.04, 0.04), baseColor.xyz, metallic);

	float3 worldPosition = ReconstructWorldPositionFromDepth(depthTex, samLinear, input.Tex, inverseViewProjection);
	float lightDistance = length(lightPosition-worldPosition);
	float3 L = normalize((lightPosition - worldPosition)/lightDistance);
	float dotNL = saturate(dot(normal, L));

	//減衰係数
    float attenuation = PhysicalAttenuation(lightAttenuation.x, lightAttenuation.y, lightAttenuation.z, lightDistance);

	//輝度
	float3 irradistance = dotNL * lightColor.xyz * attenuation;

	//ライティング済みカラー＆スペキュラ
    float3 directDiffuse = irradistance * DiffuseBRDF(diffuseColor);
    float3 directSpecular = irradistance * SpecularBRDF(normal, -input.Eye, L, specularColor, roughness);

	return float4(directDiffuse + directSpecular,1);
}