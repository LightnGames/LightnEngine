TextureCube cubeMap : register(t0);
Texture2D texDiffuse : register(t1);
Texture2D texNormal : register(t2);
Texture2D texRoughness : register(t3);
Texture2D texMetallic : register(t4);
SamplerState samLinear : register(s0);

#include "../Gbuffer.hlsl"
#include "../DeferredGeometry.hlsl"

[earlydepthstencil]
PS_OUTPUT PS(PS_INPUT input)
{
    PS_OUTPUT output;

    //�e�N�X�`�����T���v��
    float4 baseColor = texDiffuse.Sample(samLinear, input.Tex);
    float3 normal = texNormal.Sample(samLinear, input.Tex).xyz;
    float roughness = texRoughness.Sample(samLinear, input.Tex).r;
    float metallic = texMetallic.Sample(samLinear, input.Tex).r;
    //roughness = 0.5f;
    //metallic = 0;

    //baseColor.xyz = float3(1.0f, 1.0f, 1.0f);
    baseColor.xyz = lerp(float3(0.3f, 0.3f, 0.2f), baseColor.xyz, baseColor.a);
    //baseColor.a = 1;

    float flipNormal = clamp(dot(input.Normal, -input.Eye) * 100000, -1, 1);
    input.Normal *= flipNormal;
    input.Tangent *= flipNormal;
    input.Binormal *= flipNormal;

	//�m�[�}���}�b�v�����[���h�m�[�}���ɓK�p
    normal = (normal * 2.0f) - 1.0f;
    normal = (normal.x * input.Tangent) + (normal.y * input.Binormal) + (normal.z * input.Normal);
    normal = normalize(normal);
    normal = lerp(input.Normal, normal, baseColor.a);

    //output.albedo = float4(flipNormal, 0, 0, 1);
    //roughness = 1;
    output.albedo = baseColor;
    output.normal = float4(EncodeNormal(normal), 1);
    output.rme = float4(roughness, metallic, 0, 0);

    return output;
}