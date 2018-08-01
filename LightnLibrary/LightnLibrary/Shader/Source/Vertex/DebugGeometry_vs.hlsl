
//�萔�o�b�t�@
cbuffer ConstBuff : register(b0)
{
    matrix mtxProj;
    matrix mtxView;
    matrix mtxWorld;
}

//���_���Ƃ̃f�[�^
struct VS_INPUT
{
    float3 Pos : POSITION;
    float4 Col : COLOR;
    matrix instanceMatrix : MATRIX;
    uint InstanceId : SV_InstanceID;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Col : COLOR;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;

    //���_���W
    float4 worldPos = mul(float4(input.Pos, 1), input.instanceMatrix);
    //float4 worldPos = mul(float4(input.Pos, 1), mtxWorld);
    output.Pos = worldPos;
    output.Pos = mul(output.Pos, mtxView);
    output.Pos = mul(output.Pos, mtxProj);
    output.Col = input.Col;

    return output;
}