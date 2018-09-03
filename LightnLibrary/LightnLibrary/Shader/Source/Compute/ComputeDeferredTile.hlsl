struct SurfaceData
{
    float3 positionView;
    float3 normal;
    float4 albedo;
    float roughness;
    float metallic;
};

struct PointLight
{
    float3 positionView;
    float attenuationBegin;
    float3 color;
    float attenuationEnd;
};

struct SpotLight
{
    float3 positionView;
    float attenuationBegin;
    float3 color;
    float attenuationEnd;
    float4 direction;
};

Texture2DMS<float4> gBufferTextures[4] : register(t0);
StructuredBuffer<PointLight> pointLights : register(t4);
StructuredBuffer<SpotLight> spotLights : register(t5);
TextureCube cubeMap : register(t6);
RWStructuredBuffer<uint2> framebuffer : register(u0);
SamplerState samLinear : register(s0);

#include "../ShaderDefines.h"
#include "../TileBasedCullingInclude.hlsl"
#include "../PhysicallyBasedRendering.hlsl"
#include "../DeferredLight.hlsl"
#include "../Gbuffer.hlsl"

groupshared uint sMinZ;
groupshared uint sMaxZ;

// Light list for the tile
groupshared uint sTilePointLightIndices[MAX_LIGHTS];
groupshared uint sTileSpotLightIndices[MAX_LIGHTS];
groupshared uint sTileNumPointLights;
groupshared uint sTileNumSpotLights;

// List of pixels that require per-sample shading
// We encode two 16-bit x/y coordinates in one uint to save shared memory space
groupshared uint sPerSamplePixels[COMPUTE_SHADER_TILE_GROUP_SIZE];
groupshared uint sNumPerSamplePixels;


SurfaceData ComputeSurfaceDataFromGBufferSample(uint2 positionViewport)
{
    float zBuffer = gBufferTextures[0].Load(positionViewport.xy, 0).x;
    //rawData.albedo = gBufferTextures[1].SampleLevel(samLinear, positionViewport.xy, 0).xyzw;
    //rawData.normal_specular = gBufferTextures[2].SampleLevel(samLinear, positionViewport.xy,0).xyzw;
    //rawData.positionZGrad = gBufferTextures[3].SampleLevel(samLinear, positionViewport.xy,0).xy;

    float2 gBufferDim;
    uint dummy;
    gBufferTextures[0].GetDimensions(gBufferDim.x, gBufferDim.y, dummy);

    SurfaceData data=(SurfaceData)0;

    //�r���[���W�n��Z�l
    //float viewSpaceZ = cameraProj._43 / (zBuffer - cameraProj._33);
    //data.positionView = ComputePositionViewFromZ(positionScreen, zBuffer);
    float2 texCoords = positionViewport / (float2) framebufferDimensions.xy;
    data.positionView = float3(texCoords * 2 - float2(1, 1), zBuffer);
    data.positionView.y *= -1;

    float4 roughnessMetallic = gBufferTextures[3].Load(positionViewport.xy, 0).xyzw;
    data.albedo = gBufferTextures[1].Load(positionViewport.xy, 0).xyzw;
    data.normal = gBufferTextures[2].Load(positionViewport.xy, 0).xyz;
    data.roughness = roughnessMetallic.r;
    data.metallic = roughnessMetallic.g;

    data.albedo = pow(data.albedo, 2.2f);
    data.roughness = pow(data.roughness, 2.2f);

    data.normal = DecodeNormal(data.normal);

    return data;
}

void WriteSample(uint2 coords, float4 value)
{
    framebuffer[GetFramebufferSampleAddress(coords,framebufferDimensions.xy)] = PackRGBA16(value);
}

[numthreads(COMPUTE_SHADER_TILE_GROUP_DIM, COMPUTE_SHADER_TILE_GROUP_DIM, 1)]
void CS(uint3 groupId : SV_GroupID,
        uint3 dispatchThreadId : SV_DispatchThreadID,
        uint3 groupThreadId : SV_GroupThreadID)
{

    //WriteSample(dispatchThreadId.xy, float4(groupId.x % 16 / 16.0f, groupId.y % 16 / 16.0f, 0, 1.0f));
    //WriteSample(dispatchThreadId.xy, float4(dispatchThreadId.xy / (float2)framebufferDimensions.xy, 0, 1));
    //WriteSample(dispatchThreadId.xy, 0, float4(cameraNearFar.x/10000.0f, 0, 0, 1.0f));
    //framebuffer[GetFramebufferSampleAddress(dispatchThreadId.xy, framebufferDimensions.xy, 0)] = uint2(1,1);

    //return;

    uint groupIndex = groupThreadId.y * COMPUTE_SHADER_TILE_GROUP_DIM + groupThreadId.x;

    uint totalPointLights, totalSpotLights, dummy;
    pointLights.GetDimensions(totalPointLights, dummy);
    spotLights.GetDimensions(totalSpotLights, dummy);

    uint2 globalCoords = dispatchThreadId.xy;

    //GBuffer����MSAA�̃T���v����SurfaceData�ɓ˂�����
    //Gbuffer����normal�̃f�R�[�h���Coords�̓W�J�Ȃ�
    SurfaceData surfaceSample;
    surfaceSample = ComputeSurfaceDataFromGBufferSample(globalCoords);

	//�^�C�����Ƃ�Z�l�̍ŏ��E�ő�l�����߂�
    float minZSample = cameraNearFar.y;
    float maxZSample = cameraNearFar.x;

    // Avoid shading skybox/background or otherwise invalid pixels
    float4 pposition = mul(float4(surfaceSample.positionView, 1.0f), camerProjInverse);
   // pposition = float4(surfaceSample.positionView, 1.0f);
    float3 position = pposition.xyz / pposition.w;
    float viewSpaceZ = cameraNearFar.y + position.z * cameraNearFar.x;
    bool validPixel = viewSpaceZ >= cameraNearFar.x && viewSpaceZ < cameraNearFar.y;
    [flatten]
    if (validPixel)
    {
        minZSample = min(minZSample, viewSpaceZ);
        maxZSample = max(maxZSample, viewSpaceZ);
    }

	//�X���b�h�O���[�v���L��������������
    if (groupIndex == 0)
    {
        sTileNumPointLights = 0;
        sTileNumSpotLights = 0;
        sNumPerSamplePixels = 0;
        sMinZ = 0x7F7FFFFF; // Max float
        sMaxZ = 0;
    }

	//�����܂őS�X���b�h����������܂őҋ@
    GroupMemoryBarrierWithGroupSync();
    float pix = position.z;

    if (pix > sMaxZ)
    {
        InterlockedMax(sMaxZ, asuint(pix));
    }

    if (pix < sMinZ)
    {
        InterlockedMin(sMinZ, asuint(pix));
    }

    GroupMemoryBarrierWithGroupSync();

	//�ő�l�ŏ��l��Float�ŏ�����
    float minTileZ = asfloat(sMinZ);
    float maxTileZ = asfloat(sMaxZ);

    //Dispatch x=79,y=43
    //TileScale x=39.5,y=41.5
    float2 tileScale = float2(framebufferDimensions.xy) * rcp(float(2 * COMPUTE_SHADER_TILE_GROUP_DIM));
    //x=39.5 ~ -39.5, y= 41.5 ~ -41.5
    float2 tileBias = tileScale - float2(groupId.xy);

    //�^�C�����Ƃ�6�̕��ʂ��v�Z
    float4 c1 = float4(cameraProj._11 * tileScale.x, 0.0f, tileBias.x, 0.0f);
    float4 c2 = float4(0.0f, -cameraProj._22 * tileScale.y, tileBias.y, 0.0f);
    float4 c4 = float4(0.0f, 0.0f, 1.0f, 0.0f);

    // Derive frustum planes
    float4 frustumPlanes[6];
    // Sides
    frustumPlanes[0] = c4 - c1;
    frustumPlanes[1] = c1;
    frustumPlanes[2] = c4 - c2;
    frustumPlanes[3] = c2;
    // Near/far
    frustumPlanes[4] = float4(0.0f, 0.0f, 1.0f, -minTileZ);
    frustumPlanes[5] = float4(0.0f, 0.0f, -1.0f, maxTileZ);
    
    //�㉺���E�̕��ʂ𐳋K��(Near��Far�͂��łɐ��K������Ă���)
    [unroll]
    for (uint i = 0; i < 4; ++i)
    {
        frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));
    }

    //6���ʂƌ������Ă���|�C���g���C�g���W�߂�
    for (uint lightIndex = groupIndex; lightIndex < totalPointLights; lightIndex += COMPUTE_SHADER_TILE_GROUP_SIZE)
    {
        PointLight light = pointLights[lightIndex];
                
        // Cull: point light sphere vs tile frustum
        bool inFrustum = true;
        [unroll]
        for (uint i = 0; i < 6; ++i)
        {
            //���ʂƂ̋������|�C���g���C�g�̔��a�ȉ��Ȃ�Փ�
            float d = dot(frustumPlanes[i], float4(light.positionView, 1.0f));
            inFrustum = inFrustum && (d + light.attenuationEnd > 0);
        }

        //�Փ˂��Ă��郉�C�g�Ȃ烊�X�g�ɉ�����Փ˃��C�g�J�E���g���C���N�������g
        [branch]
        if (inFrustum)
        {
            uint listIndex;
            InterlockedAdd(sTileNumPointLights, 1, listIndex);
            sTilePointLightIndices[listIndex] = lightIndex;
        }
    }

    //6���ʂƌ������Ă���X�|�b�g���C�g���W�߂�
    for (uint lightIndex = groupIndex; lightIndex < totalSpotLights; lightIndex += COMPUTE_SHADER_TILE_GROUP_SIZE)
    {
        SpotLight light = spotLights[lightIndex];
                
        // Cull: point light sphere vs tile frustum
        bool inFrustum = true;
        [unroll]
        for (uint i = 0; i < 6; ++i)
        {
            //���ʂƂ̋������|�C���g���C�g�̔��a�ȉ��Ȃ�Փ�
            float d = dot(frustumPlanes[i], float4(light.positionView, 1.0f));
            inFrustum = inFrustum && (d + light.attenuationEnd > 0);
        }

        //�Փ˂��Ă��郉�C�g�Ȃ烊�X�g�ɉ�����Փ˃��C�g�J�E���g���C���N�������g
        [branch]
        if (inFrustum)
        {
            uint listIndex;
            InterlockedAdd(sTileNumSpotLights, 1, listIndex);
            sTileSpotLightIndices[listIndex] = lightIndex;
        }
    }

    //���ׂẴX���b�h�̃��C�g�J�����O��ҋ@
    GroupMemoryBarrierWithGroupSync();

    uint numPointLights = sTileNumPointLights;
    uint numSpotLights = sTileNumSpotLights;
    float4 lit = float4(0.0f, 0.0f, 0.0f, 0.0f);

    //�^�C�����𑜓x�ɂ���Ă͂͂ݏo��̂Ń`�F�b�N
    if (all(globalCoords < framebufferDimensions.xy))
    {

        float roughness = surfaceSample.roughness;
        float metallic = surfaceSample.metallic;
        float3 albedo = surfaceSample.albedo.xyz;
        float3 normal = surfaceSample.normal;
        float3 diffuseColor = lerp(albedo, float3(0.04, 0.04, 0.04), metallic);
        float3 specularColor = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

        float3 viewPosition = position;

        float3 eyeDirection;
        eyeDirection.xy = (viewPosition.xy * 2.0f) - 1.0f;
        eyeDirection.z = viewPosition.z;
        eyeDirection = normalize(eyeDirection);
        eyeDirection = mul(float4(eyeDirection, 1), cameraRotate);

        //lit += float4(numSpotLights, 0, 0, 1.0f);

        //�|�C���g���C�g
        if (numPointLights > 0)
        {
            for (uint tileLightIndex = 0; tileLightIndex < numPointLights; ++tileLightIndex)
            {
                PointLight light = pointLights[sTilePointLightIndices[tileLightIndex]];
                float lightDistance = length(light.positionView - viewPosition);
                float3 L = normalize(light.positionView - viewPosition);

                L = mul(float4(L, 1), cameraRotate);
                L = normalize(L);

                float dotNL = saturate(dot(normal, L));

	        //�����W��
                float attenuation = PhysicalAttenuation(0.00f, 0.03f, 0.03f, lightDistance);

	        //�P�x
                float lMax = max(0.0f, light.attenuationEnd - lightDistance) / light.attenuationEnd;
                float3 irradistance = dotNL * light.color * attenuation*lMax;
                //float3 irradistance = dotNL * light.color * lMax;

	        //���C�e�B���O�ς݃J���[���X�y�L����
                float3 directDiffuse = irradistance * DiffuseBRDF(diffuseColor);
                float3 directSpecular = irradistance * SpecularBRDF(normal, -eyeDirection, L, specularColor, roughness);

                //lit += float4(saturate((normal * 2.0f)-1.0f), 0);
                //lit += float4(saturate(normal), 0);
                //lit += float4(dotNL,0,0, 0);
                //lit += float4(saturate(L), 0);
                lit += float4(directDiffuse + directSpecular, 0);
            }
        }

        //�X�|�b�g���C�g
        if (numSpotLights > 0)
        {
            for (uint tileLightIndex = 0; tileLightIndex < numSpotLights; ++tileLightIndex)
            {
                SpotLight light = spotLights[sTileSpotLightIndices[tileLightIndex]];

                float lightDistance = length(light.positionView - viewPosition);
                float3 L = normalize(light.positionView - viewPosition);

                L = mul(float4(L, 1), cameraRotate);
                L = normalize(L);

                float dotNL = saturate(dot(normal, L));

	        //�����W��
                float attenuation = PhysicalAttenuation(0.0f, 0.03f, 0.03f, lightDistance);
                float lMax = max(0.0f, light.attenuationEnd - lightDistance) / light.attenuationEnd;

                float spotDot = dot(L, light.direction.xyz);
                float spotValue = smoothstep(0.5f, 0.6f, spotDot);
                attenuation *= pow(max(spotValue, 0.0), 1);

	        //�P�x
                float3 irradistance = dotNL * light.color * attenuation * lMax;

	        //���C�e�B���O�ς݃J���[���X�y�L����
                float3 directDiffuse = irradistance * DiffuseBRDF(diffuseColor);
                float3 directSpecular = irradistance * SpecularBRDF(normal, -eyeDirection, L, specularColor, roughness);

                lit += float4(directDiffuse + directSpecular, 0);
            }
        }
    }

    WriteSample(globalCoords, lit);
}