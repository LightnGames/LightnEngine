TextureCube cubeMap : register(t0);
SamplerState samLinear : register(s0);


//�s�N�Z���V�F�[�_�[��Pos��float4�_��!!!!!!!!!!
struct PS_INPUT
{
    float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};

float4 PS ( PS_INPUT input ) : SV_Target
{
    return cubeMap.SampleLevel(samLinear, input.PosL, 0);
    //return float4(0, 0, 0, 1);
}