Texture2D HDRSource : register(t0);
SamplerState samLinear : register(s0);

#include "../ShaderDefines.h"
#include "../TileBasedCullingInclude.hlsl"

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 Eye : POSITION0;
};

float4 PS(PS_INPUT input) : SV_Target
{
    return HDRSource.Sample(samLinear, input.Tex);
}