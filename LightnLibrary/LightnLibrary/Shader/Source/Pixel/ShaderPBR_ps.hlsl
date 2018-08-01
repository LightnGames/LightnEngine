TextureCube cubeMap : register(t0);
Texture2D texDiffuse : register(t1);
Texture2D texNormal : register(t2);
Texture2D texRoughness : register(t3);
Texture2D texMetallic : register(t4);
SamplerState samLinear : register(s0);

#include "PhysicallyBasedRendering.hlsl"

struct PS_INPUT {
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Binormal : BINORMAL;
	float3 Eye : POSITION0;
};

float4 PS ( PS_INPUT input ) : SV_Target
{

    //テクスチャをサンプル
    float4 baseColor = texDiffuse.Sample(samLinear, input.Tex);
    float3 normal = texNormal.Sample(samLinear, input.Tex);
    float roughness = texRoughness.Sample(samLinear, input.Tex).r;
    float metallic = texMetallic.Sample(samLinear, input.Tex);

	//return baseColor;
    //clip(baseColor.a - 0.1);

    //baseColor = float4(1, 1, 1, 1);
    //roughness /=2;
    //metallic = 0;

    //メタリックの値からテクスチャカラー
    float3 diffuseColor = lerp(baseColor.xyz, float3(0.04, 0.04, 0.04), metallic);
    float3 specularColor = lerp(float3(0.04, 0.04, 0.04), baseColor.xyz, metallic);

    //ノーマルマップをワールドノーマルに適用
    normal = (normal * 2.0f) - 1.0f;
    normal = (normal.x * input.Tangent) + (-normal.y * input.Binormal) + (normal.z * input.Normal);
    normal = normalize(normal);
    //normal = input.Normal;
	//return float4(normal,1);

    //ライトベクトルと色を定義
    float3 L = normalize(-float3(-1, -1, 0));
    float3 LColor = float3(1, 1, 1)*2;

    //平行光源ライティング
    float dotNL = saturate(dot(normal, L));
    float3 irradistance = dotNL * LColor * PI;
	//return float4(irradistance,1);

    //ライティング済みカラー＆スペキュラ
    float3 directDiffuse = irradistance * DiffuseBRDF(diffuseColor);
    float3 directSpecular = irradistance * SpecularBRDF(normal, -input.Eye, L, specularColor, roughness);
	//return float4(directSpecular,1);

    //キューブマップ
    int maxMipLevels, width, height;
    cubeMap.GetDimensions(0, width, height, maxMipLevels);

    //キューブマップテクスチャをサンプル
    float3 cubeMapDiffuse = CubemapDiffuse(normal);
    float3 cubeMapSpecular = ApproximateSpecularIBL(specularColor, roughness, normal, -input.Eye);
	//return float4(cubeMapDiffuse,1);

    float4 finalColor = float4((cubeMapDiffuse/5.0+cubeMapSpecular)+directDiffuse + directSpecular, 1);
	//finalColor=pow(finalColor,2.2);
    //return float4(directDiffuse * cubeMapColor + directSpecular, 1);


    return finalColor;
}