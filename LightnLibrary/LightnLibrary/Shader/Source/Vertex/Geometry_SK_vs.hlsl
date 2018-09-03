
//定義
#define MAX_BONE_MATRICES 255

//定数バッファ
cbuffer ConstBuff : register(b0)
{
	matrix mtxProj;
	matrix mtxView;
	matrix mtxWorld;
    float3 cameraPos;
    matrix bone[MAX_BONE_MATRICES];
}

//頂点ごとのデータ
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

//ピクセルシェーダーへ渡すデータ
struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Normal :NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float3 Eye : POSITION0;
};

//ボーンリストからインデックスの要素を持ってくる
matrix FetchBoneMatrix(uint index)
{
    return bone[index];
}

PS_INPUT VS ( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;

    //頂点座標
    float4 localPos = float4(input.Pos, 1);
    float3 localNormal = input.Normal;
    float3 localBinormal = input.Binormal;
    float3 localTangent = input.Tangent;

    //=============================================================================頂点スキニング
    float fWeight = 0.0f;
    matrix m;

    //ボーン1
    fWeight = input.boneWight.x;
    m = FetchBoneMatrix(input.boneIndex.x);
    output.Pos += fWeight * mul(localPos, m);
    output.Normal += fWeight * mul(localNormal, (float3x3) m);
    output.Binormal += fWeight * mul(localBinormal, (float3x3) m);
    output.Tangent += fWeight * mul(localTangent, (float3x3) m);

    //ボーン2
    fWeight = input.boneWight.y;
    m = FetchBoneMatrix(input.boneIndex.y);
    output.Pos += fWeight * mul(localPos, m);
    output.Normal += fWeight * mul(localNormal, (float3x3) m);
    output.Binormal += fWeight * mul(localBinormal, (float3x3) m);
    output.Tangent += fWeight * mul(localTangent, (float3x3) m);

    //ボーン3
    fWeight = input.boneWight.z;
    m = FetchBoneMatrix(input.boneIndex.z);
    output.Pos += fWeight * mul(localPos, m);
    output.Normal += fWeight * mul(localNormal, (float3x3) m);
    output.Binormal += fWeight * mul(localBinormal, (float3x3) m);
    output.Tangent += fWeight * mul(localTangent, (float3x3) m);

    //ボーン4
    fWeight = input.boneWight.w;
    m = FetchBoneMatrix(input.boneIndex.w);
    output.Pos += fWeight * mul(localPos, m);
    output.Normal += fWeight * mul(localNormal, (float3x3) m);
    output.Binormal += fWeight * mul(localBinormal, (float3x3) m);
    output.Tangent += fWeight * mul(localTangent, (float3x3) m);

    //=============================================================================各行列合成

    //output.Pos.w = 1;
    //output.Pos = localPos;
    //output.Normal = localNormal;
    //output.Tangent = localBinormal;
    //output.Binormal = localTangent;

    float4 worldPos = mul(output.Pos, mtxWorld);
    output.Pos = worldPos;
	output.Pos = mul ( output.Pos, mtxView );
	output.Pos = mul ( output.Pos, mtxProj );
	
    //視点ベクトル
    //output.Eye = normalize(worldPos.xyz - cameraPos);

    //テクスチャ
    output.Tex = input.Tex;
	
    //法線
    output.Normal = normalize(mul(output.Normal, (float3x3) mtxWorld));
    output.Tangent = normalize(mul(output.Tangent, (float3x3) mtxWorld));
    output.Binormal = normalize(mul(output.Binormal, (float3x3)mtxWorld));

	return output;
}