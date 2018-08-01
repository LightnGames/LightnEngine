struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float3 Eye : POSITION0;
};

float4 PS ( PS_INPUT input ) : SV_Target
{
	return float4(0,0,0,1);
}