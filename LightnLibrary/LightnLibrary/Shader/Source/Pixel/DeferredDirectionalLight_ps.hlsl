Texture2D<float4> gBufferTextures[4] : register(t0);
Texture2D ShadowMap : register(t4);
SamplerState samLinear : register(s0);
SamplerComparisonState ShadowSmp : register(s1);

#include "../PhysicallyBasedRendering.hlsl"
#include "../DeferredLight.hlsl"
#include "../Gbuffer.hlsl"
#include "../ScreenQuad.hlsl"

cbuffer DirectionalLightInput : register(b0){
	float4 lightDirection;
	float4 lightColor;
    float4x4 mtxViewProjection;
    float4x4 mtxShadow;
    float4 cameraPosition;
};

float4 PS(PS_INPUT_SCREEN input) : SV_Target
{
    //return float4(input.Tex, 0, 1);
    float4 baseColor;
	float4 rtMainColor;
    float3 normal;
	float roughness;
	float metallic;

    baseColor = gBufferTextures[1].Sample(samLinear, input.Tex);
    normal = gBufferTextures[2].Sample(samLinear, input.Tex).xyz;
	float4 RT2Value = gBufferTextures[3].Sample(samLinear, input.Tex);

	roughness = RT2Value.r;
	metallic = RT2Value.g;
    normal = DecodeNormal(normal);

    //スペキュラエイリアシング防止。。。TAAが実装されたら消す
    roughness = max(roughness, 0.5f);

    baseColor.xyz = pow(baseColor.xyz, 2.2f);
    roughness = pow(saturate(roughness), 2.2f);

	//clip(length(normal)-0.1);

	//メタリックの値からテクスチャカラー
    float3 diffuseColor = lerp(baseColor.xyz, float3(0.04, 0.04, 0.04), metallic);
    float3 specularColor = lerp(float3(0.04, 0.04, 0.04), baseColor.xyz, metallic);
    float3 worldPosition = ReconstructWorldPositionFromDepth(gBufferTextures[0], samLinear, input.Tex, mtxViewProjection);
    float3 eyeDir = -normalize(worldPosition - cameraPosition.xyz);

	 //ライトベクトルと色を定義
    float3 L = lightDirection.xyz;

    //平行光源ライティング
    float dotNL = saturate(dot(normal, L));
    float3 irradistance = dotNL * lightColor.xyz;
	//return float4(irradistance,1);

    //ライティング済みカラー＆スペキュラ
    float3 directDiffuse = irradistance * DiffuseBRDF(diffuseColor);
    float3 directSpecular = irradistance * SpecularBRDF(normal, eyeDir, L, specularColor, roughness);
	//return float4(directSpecular,1);

	float4 outputColor = float4(directSpecular + directDiffuse, 1);
    float4 shadowCoord = mul(float4(worldPosition, 1), mtxShadow);
    float maxDepthSlope = max(abs(ddx(shadowCoord.z)), abs(ddy(shadowCoord.z)));

    float shadowThreshold = 1.0f; // シャドウにするかどうかの閾値です.
    float bias = 0.0003f; // 固定バイアスです.
    float slopeScaledBias = 0.5f; // 深度傾斜.
    float depthBiasClamp = 0.1f; // バイアスクランプ値.

    float shadowBias = bias + slopeScaledBias * maxDepthSlope;
    shadowBias = min(shadowBias, depthBiasClamp);

    float3 shadowColor = float3(0.05f, 0.05f, 0.05f);
    shadowThreshold = ShadowMap.SampleCmpLevelZero(ShadowSmp, shadowCoord.xy, shadowCoord.z - shadowBias);
    shadowColor = lerp(shadowColor, float3(1.0f, 1.0f, 1.0f), shadowThreshold);

    outputColor.xyz *= shadowColor;
    //outputColor.xyz = normal;
    //outputColor = float4(roughness, 0, 0, 1);
    //outputColor.y = worldPosition.y;
    //outputColor.xyz = ShadowMap.Sample(samLinear, shadowCoord.xy);
    return outputColor;
}