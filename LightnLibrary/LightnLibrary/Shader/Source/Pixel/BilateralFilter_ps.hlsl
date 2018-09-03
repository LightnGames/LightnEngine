Texture2D ColorTexture : register(t0);
Texture2D DepthTexture : register(t1);
SamplerState samLinear : register(s0);

cbuffer CbBlur : register(b0)
{
    uint4 SampleCount : packoffset(c0);
    float4 Offset[16] : packoffset(c1);
};

#include "../ScreenQuad.hlsl"

//定数バッファのオフセットを利用してガウスブラーをかける
float4 PS(PS_INPUT_SCREEN input) : SV_Target
{
    float threadHold = 1.0f;
    float softThreadHold = 0.5f;
    float bloomPower = 1.5f;
    float4 result = 0;

    //float centerDepth = DepthTexture.Sample(samLinear, input.Tex).r;
    float4 centerColor = ColorTexture.Sample(samLinear, input.Tex);
    float pixScale = Offset[15].x;
    float horizontal = Offset[15].y;
    float vertical = Offset[15].z;

    uint sampleCount = SampleCount.x;

    for (int i = 0; i < sampleCount; ++i)
    {
        int offsetIndex = i - (sampleCount - 1) / 2;
        float offsetScale = offsetIndex * pixScale;
        float2 offset = float2(offsetScale * horizontal, offsetScale * vertical);
        float4 color = ColorTexture.Sample(samLinear, input.Tex + offset) * Offset[abs(offsetIndex)].x;
        result += color;
    }
    
    return result;
}