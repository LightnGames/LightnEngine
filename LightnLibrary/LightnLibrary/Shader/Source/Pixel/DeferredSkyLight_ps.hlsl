Texture2D RT0 : register(t0);
Texture2D RT1 : register(t1);
Texture2D RT2 : register(t2);
TextureCube cubeMap : register(t3);
SamplerState samLinear : register(s0);

#include "../PhysicallyBasedRendering.hlsl"
#include "../Gbuffer.hlsl"

cbuffer SkyLightInput : register(b0){
	float4 lightColor;
	float4 lightIntensity;
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
    float3 normal;
	float roughness;
	float metallic;

    baseColor = RT0.Sample(samLinear, input.Tex);
    normal = RT1.Sample(samLinear, input.Tex).xyz;
	float4 RT2Value = RT2.Sample(samLinear, input.Tex);

	roughness = RT2Value.r;
	metallic = RT2Value.g;
    normal = DecodeNormal(normal);

    baseColor.xyz = pow(baseColor.xyz, 1 / 2.2f);
    //roughness = pow(roughness, 1 / 2.2f);

	//メタリックの値からテクスチャカラー
    float3 diffuseColor = lerp(baseColor.xyz, float3(0.04, 0.04, 0.04), metallic);
    float3 specularColor = lerp(float3(0.04, 0.04, 0.04), baseColor.xyz, metallic);

    //キューブマップ
    int maxMipLevels, width, height;
    cubeMap.GetDimensions(0, width, height, maxMipLevels);

    //キューブマップテクスチャをサンプル
    float3 cubeMapDiffuse = diffuseColor * CubemapDiffuse(normal);
    float3 cubeMapSpecular = ApproximateSpecularIBL(specularColor, roughness, normal, -input.Eye);
	//return float4(cubeMapDiffuse,1);

	float3 skyColor = diffuseColor*lightColor.xyz;

    float4 outputColor = float4(cubeMapDiffuse * lightIntensity.r + cubeMapSpecular * lightIntensity.g + skyColor, 1);

    outputColor.xyz = pow(outputColor.xyz, 2.2f);
    return outputColor;
}