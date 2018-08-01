Texture2D RT0 : register(t0);
Texture2D RT1 : register(t1);
Texture2D RT2 : register(t2);
TextureCube cubeMap : register(t3);
SamplerState samLinear : register(s0);

#include "PhysicallyBasedRendering.hlsl"

cbuffer DirectionalLightInput : register(b0){
	float4 lightDirection;
	float4 lightColor;
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

	//clip(length(normal)-0.1);

	//メタリックの値からテクスチャカラー
    float3 diffuseColor = lerp(baseColor.xyz, float3(0.04, 0.04, 0.04), metallic);
    float3 specularColor = lerp(float3(0.04, 0.04, 0.04), baseColor.xyz, metallic);

	 //ライトベクトルと色を定義
    float3 L = lightDirection.xyz;

    //平行光源ライティング
    float dotNL = saturate(dot(normal, L));
    float3 irradistance = dotNL * lightColor.xyz;
	//return float4(irradistance,1);

    //ライティング済みカラー＆スペキュラ
    float3 directDiffuse = irradistance * DiffuseBRDF(diffuseColor);
    float3 directSpecular = irradistance * SpecularBRDF(normal, -input.Eye, L, specularColor, roughness);
	//return float4(directSpecular,1);

	float4 outputColor = float4(directSpecular + directDiffuse, 1);

    return outputColor;
}