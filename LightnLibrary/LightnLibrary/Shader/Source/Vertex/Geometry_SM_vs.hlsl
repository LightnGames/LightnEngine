
//�萔�o�b�t�@�@�����o�ϐ��݂����Ȃ���
cbuffer ConstBuff : register(b0)
{
    matrix mtxProj;
    matrix mtxView;
    matrix mtxWorld;
    float3 cameraPos;
}

//���_���Ƃ̃f�[�^
struct VS_INPUT
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};


struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float3 Eye : POSITION0;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;

    //���_���W
    float4 worldPos = mul(float4(input.Pos, 1), mtxWorld);
    output.Pos = worldPos;
    output.Pos = mul(output.Pos, mtxView);
    output.Pos = mul(output.Pos, mtxProj);
	
    //���_�x�N�g��
    output.Eye = normalize(worldPos.xyz - cameraPos);

    //�e�N�X�`��
    output.Tex = input.Tex;
	
	//�@��
    output.Normal = normalize(mul(input.Normal, (float3x3) mtxWorld));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) mtxWorld));
    output.Binormal = normalize(mul(input.Binormal, (float3x3) mtxWorld));

    return output;
}