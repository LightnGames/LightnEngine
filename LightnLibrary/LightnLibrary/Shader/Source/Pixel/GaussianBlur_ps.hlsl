Texture2D ColorTexture : register(t0);
SamplerState samLinear : register(s0);

cbuffer CbBlur
{
    uint4 SampleCount : packoffset(c0);
    float4 Offset[16] : packoffset(c1);
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 Eye : POSITION0;
};

//定数バッファのオフセットを利用してガウスブラーをかける
float4 PS(PS_INPUT input) : SV_Target
{
    float threadHold = 1.0f;
    float softThreadHold = 0.5f;
    float bloomPower = 1.5f;
    float4 result = 0;

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
        float knee = softThreadHold * threadHold;
        float soft = pow(min(knee * 2.0f, max(0, color - threadHold + knee)), 2.0f) / (4 * knee * 0.00001f);

        result += color;
    }

    float4 bloomSource = max(result - threadHold, 0) / max(result, 0.0001f) * bloomPower;
    result = lerp(result, bloomSource, Offset[15].w);
    result = saturate(result);
    result.a = 1.0f;

    return result;
}