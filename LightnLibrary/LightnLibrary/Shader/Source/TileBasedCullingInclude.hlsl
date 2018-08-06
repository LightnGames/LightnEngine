cbuffer PerFrameConstants : register(b0)
{
    float4x4 camerProjInverse;
    float4x4 cameraRotate;
    float4x4 cameraProj;
    float2 cameraNearFar;
    uint2 framebufferDimensions;
};

// - RGBA 16-bit per component packed into a uint2 per texel
float4 UnpackRGBA16(uint2 e)
{
    return float4(f16tof32(e), f16tof32(e >> 16));
}
uint2 PackRGBA16(float4 c)
{
    return f32tof16(c.rg) | (f32tof16(c.ba) << 16);
}

uint PackCoords(uint2 coords)
{
    return coords.y << 16 | coords.x;
}
uint2 UnpackCoords(uint coords)
{
    return uint2(coords & 0xFFFF, coords >> 16);
}

uint GetFramebufferSampleAddress(uint2 coords, uint2 gBufferScreenSize)
{
    // Major ordering: Row (x), Col (y), MSAA sample
    return (coords.y * gBufferScreenSize.x) + coords.x;
}