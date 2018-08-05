// ���͗p�̍\���̒�`
struct BoundingBox
{
    float4 extent; //And Mesh Id
    matrix mtxWorld;
};

#define THREAD_NUM 4

StructuredBuffer<BoundingBox> BufferIn : register(t0);
AppendStructuredBuffer<matrix> instanceMatrixBuffer : register(u0);
RWByteAddressBuffer instanceDrawListBuffer : register(u1);

struct Plane
{
    float4 normal;
    float4 position;
};

cbuffer FrustumInfo : register(b0)
{
    Plane planes[6];
};

float3 PositionFromMatrix(matrix mtxWorld)
{
    return float3(mtxWorld[3][0], mtxWorld[3][1], mtxWorld[3][2]);
}

float3 GetPositivePoint(BoundingBox box, float3 planeNormal)
{

    float3 result = PositionFromMatrix(box.mtxWorld) - box.extent.xyz / 2;

    if (planeNormal.x > 0)
    {
        result.x += box.extent.x;
    }
    if (planeNormal.y > 0)
    {
        result.y += box.extent.y;
    }
    if (planeNormal.z > 0)
    {
        result.z += box.extent.z;
    }

    return result;
}

groupshared uint drawListOffset;
groupshared uint materialCount;
groupshared uint instanceNum;

[numthreads(THREAD_NUM, 1, 1)]
void CS(uint3 dispatchId : SV_GroupID, uint3 groupId : SV_GroupThreadID)
{
    if (groupId.x == 0)
    {
        drawListOffset = BufferIn[0].extent.w;
        instanceNum = BufferIn[1].extent.w;
        materialCount = BufferIn[2].extent.w;
    }

    GroupMemoryBarrierWithGroupSync();

    uint meshId = dispatchId.x * THREAD_NUM + groupId.x;
    BoundingBox meshInfo = BufferIn[meshId];

    if (meshId > instanceNum)
    {
        return;
    }

    for (int i = 0; i < 6; i++)
    {
        float3 N = planes[i].normal;
        float3 P = GetPositivePoint(meshInfo, N);
        //P = PositionFromMatrix(meshInfo.mtxWorld);
        float3 PA = P - planes[i].position.xyz;

        float dotPA_N = dot(PA, N);

        if (dotPA_N < 0)
        {
            return;
        }
    }

    uint originalValue;
    instanceMatrixBuffer.Append(meshInfo.mtxWorld);


    for (int i = 0; i < materialCount; i++)
    {
        instanceDrawListBuffer.InterlockedAdd(((drawListOffset + i) * 20) + 4, 1, originalValue);
    }
}