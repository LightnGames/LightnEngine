#include "../ShaderDefines.h"
#include "../TileBasedCullingInclude.hlsl"

Texture2D HDRSource : register(t0);
texture2D GaussianDownSamples[4] : register(t1);
SamplerState samLinear : register(s0);

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 Eye : POSITION0;
};

#define A 0.15
#define B 0.50
#define C 0.10
#define D 0.20
#define E 0.02
#define F 0.30
#define W 11.2

float3 Uncharted2Tonemap(float3 x)
{
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float4 FetchColor(Texture2D map, float2 uv)
{
    float4 output = 0;
    output += map.SampleLevel(samLinear, uv, 0);
    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
    float4 texColor = HDRSource.Sample(samLinear, input.Tex);

    //ブルーム適用
    //texColor = 0;
    texColor += FetchColor(GaussianDownSamples[0], input.Tex);
    texColor += FetchColor(GaussianDownSamples[1], input.Tex);
    texColor += FetchColor(GaussianDownSamples[2], input.Tex);
    texColor += FetchColor(GaussianDownSamples[3], input.Tex);
    texColor.a = 1.0f;

    float ExposureBias = 2.0f;
    float3 curr = Uncharted2Tonemap(ExposureBias * texColor.xyz);

    float3 whiteScale = 1.0f / Uncharted2Tonemap(W);
    float3 color = curr * whiteScale;
    //color = texColor.xyz;
      
    float3 retColor = pow(color, 1 / 2.2);

    return float4(retColor.xyz, 1);
}