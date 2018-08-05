
StructuredBuffer<matrix> instanceMatrices : register(t0);

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

//頂点ごとのデータ
struct VS_INPUT
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    uint InstanceId : SV_InstanceID;
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

    //頂点座標
    float4x4 instanceMtxWorld = instanceMatrices[input.InstanceId + meshDrawOffset];
    float4 worldPos = mul(float4(input.Pos, 1), instanceMtxWorld);
    output.Pos = worldPos;
    output.Pos = mul(output.Pos, mtxView);
    output.Pos = mul(output.Pos, mtxProj);
	
    //視点ベクトル
    output.Eye = normalize(worldPos.xyz - cameraPos);

    //テクスチャ
    output.Tex = input.Tex;
	
	//法線
    output.Normal = normalize(mul(input.Normal, (float3x3) instanceMtxWorld));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) instanceMtxWorld));
    output.Binormal = normalize(mul(input.Binormal, (float3x3) instanceMtxWorld));

    return output;
}