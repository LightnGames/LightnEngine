Texture2D RT0 : register(t0);
Texture2D RT1 : register(t1);
Texture2D RT2 : register(t2);
TextureCube cubeMap : register(t3);
Texture2D depthTex : register(t4);
Texture2D ShadowMap : register(t5);
SamplerState samLinear : register(s0);
SamplerComparisonState ShadowSmp : register(s1);

#include "../PhysicallyBasedRendering.hlsl"
#include "../DeferredLight.hlsl"
#include "../Gbuffer.hlsl"

cbuffer DirectionalLightInput : register(b0){
	float4 lightDirection;
	float4 lightColor;
    matrix inverseViewProjection;
    matrix mtxShadow;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Eye : POSITION0;
};

float4 PS(PS_INPUT input) : SV_Target
{
    //return float4(input.Tex, 0, 1);
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

    baseColor.xyz = pow(baseColor.xyz, 2.2f);
    roughness = pow(roughness, 2.2f);

	//clip(length(normal)-0.1);

	//���^���b�N�̒l����e�N�X�`���J���[
    float3 diffuseColor = lerp(baseColor.xyz, float3(0.04, 0.04, 0.04), metallic);
    float3 specularColor = lerp(float3(0.04, 0.04, 0.04), baseColor.xyz, metallic);

	 //���C�g�x�N�g���ƐF���`
    float3 L = lightDirection.xyz;

    //���s�������C�e�B���O
    float dotNL = saturate(dot(normal, L));
    float3 irradistance = dotNL * lightColor.xyz;
	//return float4(irradistance,1);

    //���C�e�B���O�ς݃J���[���X�y�L����
    float3 directDiffuse = irradistance * DiffuseBRDF(diffuseColor);
    float3 directSpecular = irradistance * SpecularBRDF(normal, -input.Eye, L, specularColor, roughness);
	//return float4(directSpecular,1);

	float4 outputColor = float4(directSpecular + directDiffuse, 1);

    float3 worldPosition = ReconstructWorldPositionFromDepth(depthTex, samLinear, input.Tex, inverseViewProjection);
    float4 shadowCoord = mul(float4(worldPosition, 1), mtxShadow);
    float maxDepthSlope = max(abs(ddx(shadowCoord.z)), abs(ddy(shadowCoord.z)));

    float shadowThreshold = 1.0f; // �V���h�E�ɂ��邩�ǂ�����臒l�ł�.
    float bias = 0.0003f; // �Œ�o�C�A�X�ł�.
    float slopeScaledBias = 0.5f; // �[�x�X��.
    float depthBiasClamp = 0.1f; // �o�C�A�X�N�����v�l.

    float shadowBias = bias + slopeScaledBias * maxDepthSlope;
    shadowBias = min(shadowBias, depthBiasClamp);

    float3 shadowColor = float3(0.25f, 0.25f, 0.25f);
    shadowThreshold = ShadowMap.SampleCmpLevelZero(ShadowSmp, shadowCoord.xy, shadowCoord.z - shadowBias);
    shadowColor = lerp(shadowColor, float3(1.0f, 1.0f, 1.0f), shadowThreshold);

    outputColor.xyz *= shadowColor;
    //outputColor = float4(0, 0, 0, 1);
    //outputColor.y = worldPosition.y;
    //outputColor.xyz = ShadowMap.Sample(samLinear, shadowCoord.xy);
    return outputColor;
}