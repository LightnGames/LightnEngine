Texture2D RT0 : register(t0);
Texture2D RT1 : register(t1);
Texture2D RT2 : register(t2);
TextureCube cubeMap : register(t3);
Texture2D depthTex : register(t4);
SamplerState samLinear : register(s0);

#include "../PhysicallyBasedRendering.hlsl"
#include "../DeferredLight.hlsl"
#include "../ScreenQuad.hlsl"

cbuffer SpotLightInput : register(b0){
	float4 lightPosition;
	float4 lightDirection;
	float4 lightColor;
	float4 lightAttenuation;
	matrix inverseViewProjection;
};

float4 PS (PS_INPUT_SCREEN input ) : SV_Target
{
    float4 baseColor;
	float4 rtMainColor;
    float3 normal;
	float roughness;
	float metallic;

    baseColor = RT0.Sample(samLinear, input.Tex);
    normal = RT1.Sample(samLinear, input.Tex).xyz;
	float4 RT2Value = RT2.Sample(samLinear, input.Tex);
	roughness = RT2Value.r;
	metallic = RT2Value.g;

	//メタリックの値からテクスチャカラー
    float3 diffuseColor = lerp(baseColor.xyz, float3(0.04, 0.04, 0.04), metallic);
    float3 specularColor = lerp(float3(0.04, 0.04, 0.04), baseColor.xyz, metallic);

	float3 worldPosition = ReconstructWorldPositionFromDepth(depthTex, samLinear, input.Tex, inverseViewProjection);
	float lightDistance = length(lightPosition.xyz - worldPosition);
	float3 L = (lightPosition.xyz - worldPosition)/lightDistance;
                    
	//減衰係数
    float attenuation = PhysicalAttenuation(0, 0, lightAttenuation.z, lightDistance);  

	float spotDot = dot(-L, lightDirection.xyz);
	float spotValue = smoothstep(lightAttenuation.x,lightAttenuation.y,spotDot);
    attenuation *= pow(max(spotValue,0.0), lightDirection.w);

	float dotNL = saturate(dot(normal, L));
    float3 irradistance = dotNL * lightColor.xyz * attenuation;

	//拡散光とスペキュラ
	float3 directDiffuse = irradistance * DiffuseBRDF(diffuseColor);
    float3 directSpecular = irradistance; /* * SpecularBRDF(normal, -input.Eye, L, specularColor, roughness);*/

	return float4(directDiffuse + directSpecular,1);
}