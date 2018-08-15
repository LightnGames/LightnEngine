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
    float bloomPower = 3.0f;
    float4 result = 0;
    for (int i = 0; i < SampleCount.x; ++i)
    {
        float4 color = ColorTexture.Sample(samLinear, input.Tex + Offset[i].xy);
        float knee = softThreadHold * threadHold;
        float soft = pow(min(knee * 2.0f, max(0, color - threadHold + knee)), 2.0f) / (4 * knee * 0.00001f);
        float4 bloomSource = max(color - threadHold, 0) / max(color, 0.0001f);

        color = bloomSource * bloomPower;
        color.a = 1.0f;
        result += Offset[i].z * color;
    }

    result.a = 1.0f;

    return result;
}