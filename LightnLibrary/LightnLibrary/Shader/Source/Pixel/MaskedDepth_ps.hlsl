Texture2D texDiffuse : register(t0);
SamplerState samLinear : register(s0);

#include "../Gbuffer.hlsl"
#include "../DeferredGeometry.hlsl"

void PS(PS_INPUT input)
{

    //テクスチャをサンプル
    float4 baseColor = texDiffuse.Sample(samLinear, input.Tex);
    clip(baseColor.a - 0.1f);
}