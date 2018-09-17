StructuredBuffer<matrix> instanceMatrices : register(t0);
Texture2D heightTex : register(t1);
SamplerState samLinear : register(s0);

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

    //頂点座標
    float4x4 instanceMtxWorld = instanceMatrices[input.InstanceId + meshDrawOffset];
    float4 worldPos = mul(float4(input.Pos, 1), instanceMtxWorld);

    float2 worldCoords = worldPos.xz / 500.0f;
    float density = heightTex.SampleLevel(samLinear, worldCoords, 0).x;

    worldPos.y += density * 500;
    float3 off = float3(1.0f, 1.0f, 0.0) / 500.0f;
    float hL = heightTex.SampleLevel(samLinear, worldCoords - off.xz, 0).x * 500.0f;
    float hR = heightTex.SampleLevel(samLinear, worldCoords + off.xz, 0).x * 500.0f;
    float hD = heightTex.SampleLevel(samLinear, worldCoords - off.zy, 0).x * 500.0f;
    float hU = heightTex.SampleLevel(samLinear, worldCoords + off.zy, 0).x * 500.0f;

  // deduce terrain normal
    float3 normal;
    float3 tangent;
    float3 binormal;
    normal.x = hL - hR;
    normal.y = 1.0f;
    normal.z = hD - hU;
    normal = normalize(normal);

    tangent.x = 1.0f;
    tangent.y = hL - hR;
    tangent.z = hD - hU;
    tangent = normalize(tangent);

    binormal.x = hL - hR;
    binormal.y = hD - hU;
    binormal.z = 1.0f;
    binormal = normalize(binormal);

    output.Pos = worldPos;
    output.Pos = mul(output.Pos, mtxView);
    output.Pos = mul(output.Pos, mtxProj);
	
    //視点ベクトル
    output.Eye = normalize(worldPos.xyz - cameraPos);

    //テクスチャ
    output.Tex = input.Tex;
	
	//法線
    output.Normal = float4(normal, 1);
    output.Tangent = float4(tangent, 1);
    output.Binormal = float4(binormal, 1);

    return output;
}