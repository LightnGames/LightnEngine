StructuredBuffer<uint2> gLitTextureFlat : register(t0);
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
    uint2 coords;
    coords.x = input.Tex.x * framebufferDimensions.x;
    coords.y = input.Tex.y * framebufferDimensions.y;
    uint2 dispatchCoords = coords.xy / COMPUTE_SHADER_TILE_GROUP_DIM;
    uint2 groupCoords = coords.xy % COMPUTE_SHADER_TILE_GROUP_DIM;
    uint2 structuredCoords = dispatchCoords * COMPUTE_SHADER_TILE_GROUP_DIM + groupCoords;
    
    float3 sampleLit;
    sampleLit = UnpackRGBA16(gLitTextureFlat[GetFramebufferSampleAddress(structuredCoords, framebufferDimensions.xy)]).xyz;
    return float4(sampleLit, 0);
}