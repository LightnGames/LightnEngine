struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Col : COLOR;
};

float4 PS ( PS_INPUT input ) : SV_Target
{
    return input.Col;
    return float4(1, 0, 0, 1);

}