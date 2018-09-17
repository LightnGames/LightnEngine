#include "../DeferredGeometry.hlsl"

//�萔�o�b�t�@�@�����o�ϐ��݂����Ȃ���
cbuffer ConstBuff : register(b0)
{
    matrix mtxProj;
    matrix mtxView;
    matrix mtxWorld;
    float3 cameraPos;
}

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