float3 EncodeNormal(float3 normal)
{
    normal = normalize(normal);
    return (normal * 0.5f) + 0.5f;
}

float3 DecodeNormal(float3 normal)
{
    return (normal * 2.0f) - 1.0f;
}