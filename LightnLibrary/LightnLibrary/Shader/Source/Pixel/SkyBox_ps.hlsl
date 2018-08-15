TextureCube cubeMap : register(t0);
SamplerState samLinear : register(s0);


struct PS_INPUT
{
    float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};

float4 PS ( PS_INPUT input ) : SV_Target
{
    //return float4(1, 0, 0, 1);
    float4 color = cubeMap.SampleLevel(samLinear, input.PosL.xyz, 0);
    return pow(color, 2.2f);
    //return float4(0, 0, 0, 1);
}