StructuredBuffer<matrix> instanceMatrices : register(t0);

#include "../DeferredGeometry.hlsl"


cbuffer ConstBuff : register(b0)
{
    matrix mtxProj;
    matrix mtxView;
    matrix mtxWorld;
    float3 cameraPos;
}

cbuffer MeshDrawOffset : register(b1)
{
    uint meshDrawOffset;
    uint padding1;
    uint padding2;
    uint padding3;
}

PS_INPUT VS(VS_INSTANCED_INPUT input)
{
    PS_INPUT output;

    //���_���W
    float4x4 instanceMtxWorld = instanceMatrices[input.InstanceId + meshDrawOffset];
    float4 worldPos = mul(float4(input.Pos, 1), instanceMtxWorld);
    output.Pos = worldPos;
    output.Pos = mul(output.Pos, mtxView);
    output.Pos = mul(output.Pos, mtxProj);
	
    //���_�x�N�g��
    output.Eye = normalize(worldPos.xyz - cameraPos);

    //�e�N�X�`��
    output.Tex = input.Tex;
	
	//�@��
    output.Normal = normalize(mul(input.Normal, (float3x3) instanceMtxWorld));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) instanceMtxWorld));
    output.Binormal = normalize(mul(input.Binormal, (float3x3) instanceMtxWorld));

    return output;
}