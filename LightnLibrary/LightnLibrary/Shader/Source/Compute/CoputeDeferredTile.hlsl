struct GBuffer
{
    float4 albedo;
    float4 normal;
    float4 roughnessMetallic;
};

struct SurfaceData
{
    float3 positionView; // View space position
    float3 normal; // View space normal
    float4 albedo;
    float roughness; // Treated as a multiplier on albedo
    float metallic;
};

struct PointLight
{
    float3 positionView;
    float attenuationBegin;
    float3 color;
    float attenuationEnd;
};

Texture2DMS<float4> gBufferTextures[4] : register(t0);
StructuredBuffer<PointLight> pointLights : register(t4);
TextureCube cubeMap : register(t5);
RWStructuredBuffer<uint2> framebuffer : register(u0);
SamplerState samLinear : register(s0);

#include "../ShaderDefines.h"
#include "../TileBasedCullingInclude.hlsl"
#include "../PhysicallyBasedRendering.hlsl"
#include "../DeferredLight.hlsl"

groupshared uint sMinZ;
groupshared uint sMaxZ;

// Light list for the tile
groupshared uint sTileLightIndices[MAX_LIGHTS];
groupshared uint sTileNumLights;

// List of pixels that require per-sample shading
// We encode two 16-bit x/y coordinates in one uint to save shared memory space
groupshared uint sPerSamplePixels[COMPUTE_SHADER_TILE_GROUP_SIZE];
groupshared uint sNumPerSamplePixels;


SurfaceData ComputeSurfaceDataFromGBufferSample(uint2 positionViewport)
{
    GBuffer rawData;
    float zBuffer = gBufferTextures[0].Load(positionViewport.xy, 0).x;
    //rawData.albedo = gBufferTextures[1].SampleLevel(samLinear, positionViewport.xy, 0).xyzw;
    //rawData.normal_specular = gBufferTextures[2].SampleLevel(samLinear, positionViewport.xy,0).xyzw;
    //rawData.positionZGrad = gBufferTextures[3].SampleLevel(samLinear, positionViewport.xy,0).xy;

    float2 gBufferDim;
    uint dummy;
    gBufferTextures[0].GetDimensions(gBufferDim.x, gBufferDim.y, dummy);

    float2 screenPixelOffset = float2(2.0f, -2.0f) / gBufferDim;
    float2 positionScreen = (float2(positionViewport.xy) + 0.5f) * screenPixelOffset.xy + float2(-1.0f, 1.0);
    float2 positionScreenX = positionScreen + float2(screenPixelOffset.x, 0.0f);
    float2 positionScreenY = positionScreen + float2(0.0f, screenPixelOffset.y);

    SurfaceData data=(SurfaceData)0;

    //�r���[���W�n��Z�l
    //float viewSpaceZ = cameraProj._43 / (zBuffer - cameraProj._33);
    //data.positionView = ComputePositionViewFromZ(positionScreen, zBuffer);
    data.positionView = float3(positionScreen, zBuffer);
    data.positionView.y *= -1;

    float4 roughnessMetallic = gBufferTextures[3].Load(positionViewport.xy, 0).xyzw;
    data.albedo = gBufferTextures[1].Load(positionViewport.xy, 0).xyzw;
    data.normal = gBufferTextures[2].Load(positionViewport.xy, 0).xyz;
    data.roughness = roughnessMetallic.r;
    data.metallic = roughnessMetallic.g;

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
    //WriteSample(dispatchThreadId.xy, float4(dispatchThreadId.xy/1280.0f, 0, 1));
    //WriteSample(dispatchThreadId.xy, 0, float4(cameraNearFar.x/10000.0f, 0, 0, 1.0f));
    //framebuffer[GetFramebufferSampleAddress(dispatchThreadId.xy, framebufferDimensions.xy, 0)] = uint2(1,1);

    //return;

    uint groupIndex = groupThreadId.y * COMPUTE_SHADER_TILE_GROUP_DIM + groupThreadId.x;

    uint totalPointLights, dummy;
    pointLights.GetDimensions(totalPointLights, dummy);

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
        sTileNumLights = 0;
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

    //6���ʂƌ������Ă��郉�C�g���W�߂�
    uint totalLights = totalPointLights;
    for (uint lightIndex = groupIndex; lightIndex < totalLights; lightIndex += COMPUTE_SHADER_TILE_GROUP_SIZE)
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
            InterlockedAdd(sTileNumLights, 1, listIndex);
            sTileLightIndices[listIndex] = lightIndex;
        }
    }

    //���ׂẴX���b�h�̃��C�g�J�����O��ҋ@
    GroupMemoryBarrierWithGroupSync();

    uint numLights = sTileNumLights;
    float4 lit = float4(0.0f, 0.0f, 0.0f, 0.0f);

    //�^�C�����𑜓x�ɂ���Ă͂͂ݏo��̂Ń`�F�b�N
    if (all(globalCoords < framebufferDimensions.xy))
    {

        //lit += float4(numLights/2.0f,0, 0, 1.0f);
        if (numLights > 0)
        {
            for (uint tileLightIndex = 0; tileLightIndex < numLights; ++tileLightIndex)
            {
                PointLight light = pointLights[sTileLightIndices[tileLightIndex]];
                float attenuationEnd = light.attenuationEnd;

            //���^���b�N�̒l����e�N�X�`���J���[
                float roughness = surfaceSample.roughness;
                float metallic = surfaceSample.metallic;
                float3 albedo = surfaceSample.albedo.xyz;
                float3 normal = surfaceSample.normal;
                float3 diffuseColor = lerp(albedo, float3(0.04, 0.04, 0.04), metallic);
                float3 specularColor = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

                float3 viewPosition = position;
                viewPosition.y *= -1;

                float3 eyeDirection;
                eyeDirection.xy = (viewPosition.xy * 2.0f) - 1.0f;
                eyeDirection.z = viewPosition.z;
                eyeDirection = normalize(eyeDirection);
                eyeDirection = mul(float4(eyeDirection, 1), cameraRotate);

                float lightDistance = length(light.positionView - viewPosition);
                float3 L = normalize(light.positionView - viewPosition);

                L = mul(float4(L, 1), cameraRotate);
                L = normalize(L);

                float dotNL = saturate(dot(normal, L));

	        //�����W��
                float attenuation = PhysicalAttenuation(1, 2.0f, 0.01f, lightDistance);
                float d = saturate(attenuationEnd - lightDistance);

	        //�P�x
                float3 irradistance = dotNL * light.color * attenuation*d;

	        //���C�e�B���O�ς݃J���[���X�y�L����
                float3 directDiffuse = irradistance * DiffuseBRDF(diffuseColor);
                float3 directSpecular = irradistance * SpecularBRDF(normal, -eyeDirection, L, specularColor, roughness);

                lit += float4(directDiffuse + directSpecular, 1);
            }
        }
    }

    WriteSample(globalCoords, lit);
}