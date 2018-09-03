Texture2D DepthTex : register(t0);
Texture2D Normal : register(t1);
SamplerState samLinear : register(s0);

#include "../DeferredLight.hlsl"
#include "../GBuffer.hlsl"
#include "../ScreenQuad.hlsl"

cbuffer PostProcess : register(b0)
{
    float4x4 mtxViewProjInverse;
    float4x4 mtxViewProj;
}

float tangent(float3 p, float3 s)
{
    return (p.z - s.z) / length(p.xy - s.xy);
}

float rand(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898f, 78.233f))) * 43758.5453f);
}

#define SSAO_SAMPLE_COUNT 6

float4 PS(PS_INPUT_SCREEN input) : SV_Target
{
    //UE4 SSAO
    float3 normal = Normal.Sample(samLinear, input.Tex).xyz;
    normal = DecodeNormal(normal);
    float depth = DepthTex.Sample(samLinear, input.Tex).r;
    float3 worldPos = ReconstructWorldPositionFromDepthValue(depth, input.Tex, mtxViewProjInverse);
    float3 localPos = float3(input.Tex.xy, depth);
    float3 projPos = float3(((input.Tex * 2.0f) - 1.0f), depth);

    const float2 samples[SSAO_SAMPLE_COUNT] = { float2(0.7f, 0.7f), float2(0.7f, -0.7f), float2(0.0f, 1.0f), float2(0.3f, -0.9f), float2(1.0f, 0.0f), float2(0.6f, 0.3f) };
    
   
    float2 projBias;
    projBias.x = (1.0f / mtxViewProj._m22) * (projPos.x / depth) * (mtxViewProj._m22 / mtxViewProj._m11);
    projBias.y = (1.0f / mtxViewProj._m22) * (projPos.y / depth);

    float EdgeBias = 1.5f;
    float NormalBias = 0.0f;
    float d = 0.0f;

    for (int i = 0; i < SSAO_SAMPLE_COUNT; ++i)
    {
        float2 offset = samples[i];
        //offset.x = rand(samples[i] + input.Tex);
        //offset.y = rand(input.Tex - samples[i]);
        offset *= projBias * 0.03f;

        float2 sampleUvA = input.Tex + offset;
        float sampleA = DepthTex.Sample(samLinear, sampleUvA).r;
        float3 pl = float3(sampleUvA.xy, sampleA);
        float3 pl2 = ReconstructWorldPositionFromDepthValue(sampleA, sampleUvA, mtxViewProjInverse);
        float tl = atan(tangent(localPos, pl));

        float dotPl = saturate(dot(normal, pl2 - worldPos));
        if ((length(pl2 - worldPos) > EdgeBias))
        {
            continue;
        }

        float2 sampleUvB = input.Tex - offset;
        float sampleB = DepthTex.Sample(samLinear, sampleUvB).r;
        float3 pr = float3(sampleUvB.xy, sampleB);
        float3 pr2 = ReconstructWorldPositionFromDepthValue(sampleB, sampleUvB, mtxViewProjInverse);
        float tr = atan(tangent(localPos, pr));

        float dotPr = saturate(dot(normal, pr2 - worldPos));
        if ((length(pr2 - worldPos) > EdgeBias))
        {
            continue;
        }

        d += saturate((dotPl + dotPr) / 2.0f);
    }

    d = saturate(d / (float) SSAO_SAMPLE_COUNT) * saturate(((1.0f - depth)) *100);

    float col = (d * 20);
    return float4(col,0,0, 0);
}