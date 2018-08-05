StructuredBuffer<uint2> gLitTextureFlat : register(t0);
SamplerState samLinear : register(s0);

#define COMPUTE_SHADER_TILE_GROUP_DIM 16

#include "../Rendering.hlsl"

cbuffer PerFrameConstants : register(b0)
{
    float4x4 mCameraWorldViewProj;
    float4x4 mCameraWorldView;
    float4x4 mCameraViewProj;
    float4x4 mCameraProj;
    float4 mCameraNearFar;
    uint4 mFramebufferDimensions;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 Eye : POSITION0;
};

float4 PS(PS_INPUT input) : SV_Target
{
    uint2 coords;
    coords.x = input.Tex.x * mFramebufferDimensions.x;
    coords.y = input.Tex.y * mFramebufferDimensions.y;
    uint2 dispatchCoords = coords.xy / COMPUTE_SHADER_TILE_GROUP_DIM;
    uint2 groupCoords = coords.xy % COMPUTE_SHADER_TILE_GROUP_DIM;
    uint2 structuredCoords = dispatchCoords * COMPUTE_SHADER_TILE_GROUP_DIM + groupCoords;
    
    float3 sampleLit;
    sampleLit = UnpackRGBA16(gLitTextureFlat[GetFramebufferSampleAddress(structuredCoords, mFramebufferDimensions.xy)]).xyz;

    return float4(sampleLit, 1);
}