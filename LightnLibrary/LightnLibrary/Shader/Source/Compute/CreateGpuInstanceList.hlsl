RWByteAddressBuffer instanceDrawListBuffer : register(u1);

[numthreads(1, 1, 1)]
void CS(uint3 dispatchId : SV_GroupID, uint3 groupId : SV_GroupThreadID)
{
    uint meshId = dispatchId.x;
    instanceDrawListBuffer.Store(meshId * 20 + 4, 0);

}