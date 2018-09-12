Texture2D<float4> gBufferTextures[4] : register(t0);
TextureCube cubeMap : register(t4);
SamplerState samLinear : register(s0);

#include "../PhysicallyBasedRendering.hlsl"
#include "../Gbuffer.hlsl"
#include "../ScreenQuad.hlsl"
#include "../DeferredLight.hlsl"

cbuffer SkyLightInput : register(b0){
	float4 lightColor;
	float4 lightIntensity;
    float4x4 inverseViewProjection;
    float4 cameraPosition;
};

//キューブマップから環境スペキュラ色を参照
float3 PrefilterEnvMap(float Roughness, float3 R)
{
    float TotalWeight = 0;

    float3 N = R;
    float3 V = R;

    float3 PrefilteredColor = 0;
    const uint NumSamples = 8;

    int maxMipLevels, width, height;
    cubeMap.GetDimensions(0, width, height, maxMipLevels);

    for (uint i = 0; i < NumSamples; i++)
    {

        float2 Xi = Hammersley(i, NumSamples);

        float3 H = ImportanceSampleGGX(Xi, Roughness, N);

        float3 L = 2 * dot(V, H) * H - V;

        float NoL = saturate(dot(N, L));
        if (NoL > 0)
        {
            PrefilteredColor += cubeMap.SampleLevel(samLinear, L, Roughness * maxMipLevels).rgb * NoL;
            TotalWeight += NoL;
        }
    }

    PrefilteredColor /= TotalWeight;
    PrefilteredColor = pow(abs(PrefilteredColor), 2.2f);
    return PrefilteredColor;
}

float3 CubemapDiffuse(float3 N)
{

    int maxMipLevels, width, height;
    cubeMap.GetDimensions(0, width, height, maxMipLevels);

    float3 PrefilteredColor = cubeMap.SampleLevel(samLinear, N, maxMipLevels).rgb;
    PrefilteredColor = pow(abs(PrefilteredColor), 2.2f);
    return PrefilteredColor;

}

//キューブマップから環境色を参照
//ここは将来的にプリコンピュートする
float2 IntegrateBRDF(float Roughness, float NoV)
{

    float3 V;
    V.x = sqrt(1.0f - NoV * NoV); // sin
    V.y = 0;
    V.z = NoV; // cos

    float A = 0;
    float B = 0;

    const uint NumSamples = 8;
    for (uint i = 0; i < NumSamples; i++)
    {
        float2 Xi = Hammersley(i, NumSamples);
        float3 H = ImportanceSampleGGX(Xi, Roughness, float3(0, 0, 1));
        float3 L = 2 * dot(V, H) * H - V;

        float NoL = saturate(L.z);
        float NoH = saturate(H.z);
        float VoH = saturate(dot(V, H));

        if (NoL > 0)
        {
            float G = G_Smith(Roughness, NoV, NoL);
            float G_Vis = G * VoH / (NoH * NoV);

            float Fc = pow(1 - VoH, 5);
            A += (1 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }

    return float2(A, B) / NumSamples;
}

//総合IBLライティングカラーを取得
float3 ApproximateSpecularIBL(float3 SpecularColor, float Roughness, float3 N, float3 V)
{

    float NoV = dot(N, V);
    float3 R = normalize(2 * dot(V, N) * N - V);
    float3 PrefilteredColor = PrefilterEnvMap(Roughness, R);
    float2 EnvBRDF = IntegrateBRDF(Roughness, NoV);

    return PrefilteredColor * (SpecularColor * EnvBRDF.x + EnvBRDF.y);
}

[earlydepthstencil]
float4 PS(PS_INPUT_SCREEN input) : SV_Target
{
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

    baseColor.xyz = pow(abs(baseColor.xyz), 2.2f);
    roughness = pow(abs(roughness), 2.2f);
    //roughness = 0;

	//メタリックの値からテクスチャカラー
    float3 diffuseColor = lerp(baseColor.xyz, float3(0.04, 0.04, 0.04), metallic);
    float3 specularColor = lerp(float3(0.04, 0.04, 0.04), baseColor.xyz, metallic);

    float3 worldPosition = ReconstructWorldPositionFromDepth(gBufferTextures[0], samLinear, input.Tex, inverseViewProjection);
    float3 eyeDir = -normalize(worldPosition - cameraPosition.xyz);

    //キューブマップ
    int maxMipLevels, width, height;
    cubeMap.GetDimensions(0, width, height, maxMipLevels);

    //キューブマップテクスチャをサンプル
    float3 cubeMapDiffuse = CubemapDiffuse(normal);
    float3 cubeMapSpecular = ApproximateSpecularIBL(specularColor, roughness, normal, eyeDir);
    //return float4(cubeMapDiffuse * lightIntensity.r, 1);

    float3 skyColor = diffuseColor * lightColor.xyz;
    cubeMapDiffuse *= skyColor;

    float4 outputColor = float4(cubeMapDiffuse * lightIntensity.r + cubeMapSpecular * lightIntensity.g, 1);
    //outputColor.xyz = float3(0,0,0);
    //outputColor.a = 1;
    //outputColor.x = dot(normal, eyeDir);

    return outputColor;
}