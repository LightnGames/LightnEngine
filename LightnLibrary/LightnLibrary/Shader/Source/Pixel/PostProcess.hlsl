#include "../ShaderDefines.h"
#include "../TileBasedCullingInclude.hlsl"

#define BLOOM_SAMPLE 4

Texture2D HDRSource : register(t0);
texture2D GaussianDownSamples[BLOOM_SAMPLE] : register(t1);
Texture2D DepthTex : register(t5);
Texture2D Normal : register(t6);
Texture2D SSAO : register(t7);
SamplerState samLinear : register(s0);

#include "../DeferredLight.hlsl"
#include "../GBuffer.hlsl"

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 Eye : POSITION0;
};

cbuffer PostProcess : register(b0)
{
    float4x4 mtxViewProjInverse;
    float4x4 mtxViewProj;
}

#define VIGNETTE_POWER 1.5f
#define VIGNETTE_SLOPE 2.0f

#define A2 2.51f
#define B2 0.03f
#define C2 2.43f
#define D2 0.59f
#define E2 0.14f

float3 Uncharted2Tonemap(float3 x)
{
    return (x * x * A2 + x * B2) / ((x * x * C2 + x * D2) + E2);
}

float4 FetchColor(Texture2D map, float2 uv)
{
    return map.SampleLevel(samLinear, uv, 0);
}

float4 PS(PS_INPUT input) : SV_Target
{
    float4 texColor = HDRSource.Sample(samLinear, input.Tex);

    //ブルーム適用
    float4 bloom = 0;

    [unroll]
    for (uint i = 0; i < BLOOM_SAMPLE; i++)
    {
        Texture2D tex = GaussianDownSamples[i];
        bloom += FetchColor(tex, input.Tex);
    }

    bloom /= BLOOM_SAMPLE;

    bloom.a = 1.0f;
    texColor += bloom;


    //Tonemap
    float ExposureBias = 2.0f;
    float3 color = Uncharted2Tonemap(ExposureBias * texColor.xyz);
    //color = texColor.xyz;
    float3 retColor = pow(abs(color), 1 / 2.2);

    float ssao = SSAO.Sample(samLinear, input.Tex).r;
    retColor.xyz = lerp(retColor.xyz, float3(0, 0, 0), ssao*0.6f);

    //Vignette
    float2 tc = input.Tex - float2(0.5f, 0.5f);
    float v = dot(tc, tc);
    retColor.rgb *= (1.0 - pow(v, VIGNETTE_SLOPE * 0.5f) * VIGNETTE_POWER);

    return float4(retColor.xyz, 1);
}