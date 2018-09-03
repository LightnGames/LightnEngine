
//��`
#define MAX_BONE_MATRICES 255

//�萔�o�b�t�@
cbuffer ConstBuff : register(b0)
{
	matrix mtxProj;
	matrix mtxView;
	matrix mtxWorld;
    float3 cameraPos;
    matrix bone[MAX_BONE_MATRICES];
}

//���_���Ƃ̃f�[�^
struct VS_INPUT
{
	float3 Pos    : POSITION;
	float2 Tex    : TEXCOORD0;
	float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    uint4 boneIndex : BONE_INDEX;
    float4 boneWight : BONE_WEIGHT;
};

//�s�N�Z���V�F�[�_�[�֓n���f�[�^
struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Normal :NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float3 Eye : POSITION0;
};

//�{�[�����X�g����C���f�b�N�X�̗v�f�������Ă���
matrix FetchBoneMatrix(uint index)
{
    return bone[index];
}

PS_INPUT VS ( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;

    //���_���W
    float4 localPos = float4(input.Pos, 1);
    float3 localNormal = input.Normal;
    float3 localBinormal = input.Binormal;
    float3 localTangent = input.Tangent;

    //=============================================================================���_�X�L�j���O
    float fWeight = 0.0f;
    matrix m;

    //�{�[��1
    fWeight = input.boneWight.x;
    m = FetchBoneMatrix(input.boneIndex.x);
    output.Pos += fWeight * mul(localPos, m);
    output.Normal += fWeight * mul(localNormal, (float3x3) m);
    output.Binormal += fWeight * mul(localBinormal, (float3x3) m);
    output.Tangent += fWeight * mul(localTangent, (float3x3) m);

    //�{�[��2
    fWeight = input.boneWight.y;
    m = FetchBoneMatrix(input.boneIndex.y);
    output.Pos += fWeight * mul(localPos, m);
    output.Normal += fWeight * mul(localNormal, (float3x3) m);
    output.Binormal += fWeight * mul(localBinormal, (float3x3) m);
    output.Tangent += fWeight * mul(localTangent, (float3x3) m);

    //�{�[��3
    fWeight = input.boneWight.z;
    m = FetchBoneMatrix(input.boneIndex.z);
    output.Pos += fWeight * mul(localPos, m);
    output.Normal += fWeight * mul(localNormal, (float3x3) m);
    output.Binormal += fWeight * mul(localBinormal, (float3x3) m);
    output.Tangent += fWeight * mul(localTangent, (float3x3) m);

    //�{�[��4
    fWeight = input.boneWight.w;
    m = FetchBoneMatrix(input.boneIndex.w);
    output.Pos += fWeight * mul(localPos, m);
    output.Normal += fWeight * mul(localNormal, (float3x3) m);
    output.Binormal += fWeight * mul(localBinormal, (float3x3) m);
    output.Tangent += fWeight * mul(localTangent, (float3x3) m);

    //=============================================================================�e�s�񍇐�

    //output.Pos.w = 1;
    //output.Pos = localPos;
    //output.Normal = localNormal;
    //output.Tangent = localBinormal;
    //output.Binormal = localTangent;

    float4 worldPos = mul(output.Pos, mtxWorld);
    output.Pos = worldPos;
	output.Pos = mul ( output.Pos, mtxView );
	output.Pos = mul ( output.Pos, mtxProj );
	
    //���_�x�N�g��
    //output.Eye = normalize(worldPos.xyz - cameraPos);

    //�e�N�X�`��
    output.Tex = input.Tex;
	
    //�@��
    output.Normal = normalize(mul(output.Normal, (float3x3) mtxWorld));
    output.Tangent = normalize(mul(output.Tangent, (float3x3) mtxWorld));
    output.Binormal = normalize(mul(output.Binormal, (float3x3)mtxWorld));

	return output;
}