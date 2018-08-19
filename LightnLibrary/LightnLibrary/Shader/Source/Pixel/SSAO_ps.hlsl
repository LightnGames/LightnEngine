Texture2D DepthTex : register(t0);
Texture2D Normal : register(t1);
SamplerState samLinear : register(s0);

#include "../DeferredLight.hlsl"
#include "../GBuffer.hlsl"

cbuffer PostProcess : register(b0)
{
    float4x4 mtxViewProjInverse;
    float4x4 mtxViewProj;
}

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 Eye : POSITION0;
};

float tangent(float3 p, float3 s)
{
    return (p.z - s.z) / length(p.xy - s.xy);
}

#define SSAO_SAMPLE_COUNT 6

float4 PS(PS_INPUT input) : SV_Target
{
      //UE4 SSAO
    float3 normal = Normal.Sample(samLinear, input.Tex);
    normal = DecodeNormal(normal);
    float depth = DepthTex.Sample(samLinear, input.Tex).r;
    float3 worldPos = ReconstructWorldPositionFromDepthValue(depth, input.Tex, mtxViewProjInverse);
    float3 localPos = float3(input.Tex.xy, depth);
    float3 projPos = float3(((input.Tex * 2.0f) - 1.0f), depth);

    float2 samples[SSAO_SAMPLE_COUNT] = { float2(0.7f, 0.7f), float2(0.7f, -0.7f), float2(0.0f, 1.0f), float2(0.3f, -0.9f), float2(1.0f, 0.0f), float2(0.6f, 0.3f) };
    
    float2 projBias;
    projBias.x = (1.0f / mtxViewProj._m22) * (projPos.x / depth) * (mtxViewProj._m22 / mtxViewProj._m11);
    projBias.y = (1.0f / mtxViewProj._m22) * (projPos.y / depth);

    float EdgeBias = 0.0015f;
    float NormalBias = 0.02f;
    float d = 0.0f;

    for (int i = 0; i < SSAO_SAMPLE_COUNT; ++i)
    {
        float2 offset = samples[i] * projBias * 0.05f;

        float2 sampleUvA = input.Tex + offset;
        float sampleA = DepthTex.Sample(samLinear, sampleUvA).r;
        float3 pl = float3(sampleUvA.xy, sampleA);
        float3 pl2 = ReconstructWorldPositionFromDepthValue(sampleA, sampleUvA, mtxViewProjInverse);
        float tl = atan(tangent(localPos, pl));

        float dotPl = dot(normal, pl2 - worldPos);
        if ((sampleA - depth + EdgeBias < 0) || (dotPl + NormalBias < 0))
        {
            continue;
        }

        float2 sampleUvB = input.Tex - offset;
        float sampleB = DepthTex.Sample(samLinear, sampleUvB).r;
        float3 pr = float3(sampleUvB.xy, sampleB);
        float3 pr2 = ReconstructWorldPositionFromDepthValue(sampleB, sampleUvB, mtxViewProjInverse);
        float tr = atan(tangent(localPos, pr));

        float dotPr = dot(normal, pr2 - worldPos);
        if ((sampleB - depth + EdgeBias < 0) || (dotPr + NormalBias < 0))
        {
            continue;
        }

        d += saturate((tl + tr) / 3.141592f);
    }

    d = saturate(d / (float) SSAO_SAMPLE_COUNT);

    float col = (d * 50);
    return float4(col, 0, 0, 0);
}