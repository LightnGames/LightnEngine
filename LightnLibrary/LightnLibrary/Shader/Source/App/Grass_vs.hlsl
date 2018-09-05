#include "../DeferredGeometry.hlsl"

StructuredBuffer<matrix> instanceMatrices : register(t0);

//定数バッファ　メンバ変数みたいなもん
cbuffer ConstBuff : register(b0)
{
    float4x4 mtxProj;
    float4x4 mtxView;
    float4x4 mtxWorld;
    float3 cameraPos;
}

cbuffer MeshDrawOffset : register(b1)
{
    uint meshDrawOffset;
    uint padding1;
    uint padding2;
    uint padding3;
}

cbuffer WorldBuff : register(b2)
{
    float4 time;
    float4 playerPos;
}

struct VS_INPUT_INSTANCE
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    uint InstanceId : SV_InstanceID;
};

PS_INPUT VS(VS_INPUT_INSTANCE input)
{
    PS_INPUT output;

    //頂点座標
    float4x4 instanceMtxWorld = instanceMatrices[input.InstanceId + meshDrawOffset];
    float4 worldPos = mul(float4(input.Pos, 1), instanceMtxWorld);

    //草を揺らす
    float heightAlpha = worldPos.y / 5.0f;
    worldPos.z += sin(time.x + worldPos.z) * heightAlpha;
    worldPos.x += cos(time.x + worldPos.x) * heightAlpha * 0.5f;

    float l = length(worldPos.xyz - playerPos.xyz);
    float3 velocity = (worldPos.xyz - playerPos.xyz) / l;
    l = 1 - saturate((l - 1)*0.5f);
    worldPos.xz += velocity.xz * l * worldPos.y;

    output.Pos = worldPos;
    output.Pos = mul(output.Pos, mtxView);
    output.Pos = mul(output.Pos, mtxProj);
	
    //視点ベクトル
    output.Eye = normalize(worldPos.xyz - cameraPos);

    //テクスチャ
    output.Tex = input.Tex;
	
	//法線
    output.Normal = normalize(mul(input.Normal, (float3x3) mtxWorld));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) mtxWorld));
    output.Binormal = normalize(mul(input.Binormal, (float3x3) mtxWorld));

    return output;
}