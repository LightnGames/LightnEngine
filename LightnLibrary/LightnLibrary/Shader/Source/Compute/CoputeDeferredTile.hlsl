#define COMPUTE_SHADER_TILE_GROUP_DIM 16
#define COMPUTE_SHADER_TILE_GROUP_SIZE (COMPUTE_SHADER_TILE_GROUP_DIM*COMPUTE_SHADER_TILE_GROUP_DIM)
#define MSAA_SAMPLES 4
#define MAX_LIGHTS 256
#define DEFER_PER_SAMPLE 1

struct GBuffer
{
    float4 normal_specular : SV_Target0;
    float4 albedo : SV_Target1;
    float2 positionZGrad : SV_Target2;
};

struct SurfaceData
{
    float3 positionView; // View space position
    float3 positionViewDX; // Screen space derivatives
    float3 positionViewDY; // of view space position
    float3 normal; // View space normal
    float4 albedo;
    float specularAmount; // Treated as a multiplier on albedo
    float specularPower;
};

struct PointLight
{
    float3 positionView;
    float attenuationBegin;
    float3 color;
    float attenuationEnd;
};

cbuffer PerFrameConstants : register(b0)
{
    float4x4 mCameraWorldViewProj;
    float4x4 mCameraWorldView;
    float4x4 mCameraViewProj;
    float4x4 mCameraProj;
    float4 mCameraNearFar;
    uint4 mFramebufferDimensions;
};

Texture2DMS<float4, MSAA_SAMPLES> gBufferTextures[4] : register(t0);
StructuredBuffer<PointLight> pointLights : register(t1);
RWStructuredBuffer<uint2> framebuffer : register(u0);

groupshared uint sMinZ;
groupshared uint sMaxZ;

// Light list for the tile
groupshared uint sTileLightIndices[MAX_LIGHTS];
groupshared uint sTileNumLights;

// List of pixels that require per-sample shading
// We encode two 16-bit x/y coordinates in one uint to save shared memory space
groupshared uint sPerSamplePixels[COMPUTE_SHADER_TILE_GROUP_SIZE];
groupshared uint sNumPerSamplePixels;

float3 DecodeSphereMap(float2 e)
{
    float2 tmp = e - e * e;
    float f = tmp.x + tmp.y;
    float m = sqrt(4.0f * f - 1.0f);
    
    float3 n;
    n.xy = m * (e * 4.0f - 2.0f);
    n.z = 3.0f - 8.0f * f;
    return n;
}

float3 ComputePositionViewFromZ(float2 positionScreen, float viewSpaceZ)
{
    float2 screenSpaceRay = float2(positionScreen.x / mCameraProj._11,
                                positionScreen.y / mCameraProj._22);

    float3 positionView;
    positionView.z = viewSpaceZ;
    positionView.xy = screenSpaceRay.xy * positionView.z;

    return positionView;
}

SurfaceData ComputeSurfaceDataFromGBufferSample(uint2 positionViewport, uint sampleIndex)
{
    GBuffer rawData;
    rawData.normal_specular = gBufferTextures[0].Load(positionViewport.xy, sampleIndex).xyzw;
    rawData.albedo = gBufferTextures[1].Load(positionViewport.xy, sampleIndex).xyzw;
    rawData.positionZGrad = gBufferTextures[2].Load(positionViewport.xy, sampleIndex).xy;
    float zBuffer = gBufferTextures[3].Load(positionViewport.xy, sampleIndex).x;

    float2 gBufferDim;
    uint dummy;
    gBufferTextures[0].GetDimensions(gBufferDim.x, gBufferDim.y, dummy);

    float2 screenPixelOffset = float2(2.0f, -2.0f) / gBufferDim;
    float2 positionScreen = (float2(positionViewport.xy) + 0.5f) * screenPixelOffset.xy + float2(-1.0f, 1.0);
    float2 positionScreenX = positionScreen + float2(screenPixelOffset.x, 0.0f);
    float2 positionScreenY = positionScreen + float2(0.0f, screenPixelOffset.y);

    SurfaceData data;

    //ビュー座標系のZ値
    float viewSpaceZ = mCameraProj._43 / (zBuffer - mCameraProj._33);

    data.positionView = ComputePositionViewFromZ(positionScreen, viewSpaceZ);
    data.positionViewDX = ComputePositionViewFromZ(positionScreenX, viewSpaceZ + rawData.positionZGrad.x);
    data.positionViewDY = ComputePositionViewFromZ(positionScreenY, viewSpaceZ + rawData.positionZGrad.y);

    data.normal = DecodeSphereMap(rawData.normal_specular.xy);
    data.albedo = rawData.albedo;
    data.specularAmount = rawData.normal_specular.z;
    data.specularPower = rawData.normal_specular.w;

    return data;
}

void ComputeSurfaceDataFromGBufferAllSamples(uint2 positionViewport, out SurfaceData surface[MSAA_SAMPLES])
{
    [unroll]
    for (uint i = 0; i < MSAA_SAMPLES; ++i)
    {
        surface[i] = ComputeSurfaceDataFromGBufferSample(positionViewport, i);
    }
}

// - RGBA 16-bit per component packed into a uint2 per texel
float4 UnpackRGBA16(uint2 e)
{
    return float4(f16tof32(e), f16tof32(e >> 16));
}
uint2 PackRGBA16(float4 c)
{
    return f32tof16(c.rg) | (f32tof16(c.ba) << 16);
}

uint PackCoords(uint2 coords)
{
    return coords.y << 16 | coords.x;
}
uint2 UnpackCoords(uint coords)
{
    return uint2(coords & 0xFFFF, coords >> 16);
}

uint GetFramebufferSampleAddress(uint2 coords, uint sampleIndex)
{
    // Major ordering: Row (x), Col (y), MSAA sample
    return (sampleIndex * mFramebufferDimensions.y + coords.y) * mFramebufferDimensions.x + coords.x;
}

void WriteSample(uint2 coords, uint sampleIndex, float4 value)
{
    framebuffer[GetFramebufferSampleAddress(coords, sampleIndex)] = PackRGBA16(value);
}

// Check if a given pixel can be shaded at pixel frequency (i.e. they all come from
// the same surface) or if they require per-sample shading
bool RequiresPerSampleShading(SurfaceData surface[MSAA_SAMPLES])
{
    bool perSample = false;

    const float maxZDelta = abs(surface[0].positionViewDX.z) + abs(surface[0].positionViewDY.z);
    const float minNormalDot = 0.99f; // Allow ~8 degree normal deviations

    [unroll]
    for (uint i = 1; i < MSAA_SAMPLES; ++i)
    {
        // Using the position derivatives of the triangle, check if all of the sample depths
        // could possibly have come from the same triangle/surface
        perSample = perSample ||
            abs(surface[i].positionView.z - surface[0].positionView.z) > maxZDelta;

        // Also flag places where the normal is different
        perSample = perSample ||
            dot(surface[i].normal, surface[0].normal) < minNormalDot;
    }

    return perSample;
}

[numthreads(COMPUTE_SHADER_TILE_GROUP_DIM, COMPUTE_SHADER_TILE_GROUP_DIM, 1)]
void CS(uint3 groupId : SV_GroupID,
        uint3 dispatchThreadId : SV_DispatchThreadID,
        uint3 groupThreadId : SV_GroupThreadID)
{
    uint groupIndex = groupThreadId.y * COMPUTE_SHADER_TILE_GROUP_DIM + groupThreadId.x;

    uint totalPointLights, dummy;
    pointLights.GetDimensions(totalPointLights, dummy);

    uint2 globalCoords = dispatchThreadId.xy;

    //GBufferからMSAAのサンプル数SurfaceDataに突っ込む
    //GbufferからnormalのデコードやらCoordsの展開など
    SurfaceData surfaceSamples[MSAA_SAMPLES];
    ComputeSurfaceDataFromGBufferAllSamples(globalCoords, surfaceSamples);

	//タイルごとのZ値の最小・最大値を求める
    float minZSample = mCameraNearFar.y;
    float maxZSample = mCameraNearFar.x;
	{
		[unroll]
        for (uint sample = 0; sample < MSAA_SAMPLES; ++sample)
        {
			// Avoid shading skybox/background or otherwise invalid pixels
            float viewSpaceZ = surfaceSamples[sample].positionView.z;
            bool validPixel =
				viewSpaceZ >= mCameraNearFar.x &&
				viewSpaceZ < mCameraNearFar.y;
			[flatten]
            if (validPixel)
            {
                minZSample = min(minZSample, viewSpaceZ);
                maxZSample = max(maxZSample, viewSpaceZ);
            }
        }
    }

	//スレッドグループ共有メモリを初期化
    if (groupIndex == 0)
    {
        sTileNumLights = 0;
        sNumPerSamplePixels = 0;
        sMinZ = 0x7F7FFFFF; // Max float
        sMaxZ = 0;
    }

	//ここまで全スレッドが完了するまで待機
    GroupMemoryBarrierWithGroupSync();

	//最大Zが最小Zより大きければ値をセット(そうでなければ初期化情報のまま)
    if (maxZSample >= minZSample)
    {
        InterlockedMin(sMinZ, asuint(minZSample));
        InterlockedMax(sMaxZ, asuint(maxZSample));
    }

    GroupMemoryBarrierWithGroupSync();

	//最大値最小値をFloatで初期化
    float minTileZ = asfloat(sMinZ);
    float maxTileZ = asfloat(sMaxZ);

    //謎のバイアス？
    float2 tileScale = float2(mFramebufferDimensions.xy) * rcp(float(2 * COMPUTE_SHADER_TILE_GROUP_DIM));
    float2 tileBias = tileScale - float2(groupId.xy);

    //タイルごとの6つの平面を計算
    float4 c1 = float4(mCameraProj._11 * tileScale.x, 0.0f, tileBias.x, 0.0f);
    float4 c2 = float4(0.0f, -mCameraProj._22 * tileScale.y, tileBias.y, 0.0f);
    float4 c4 = float4(0.0f, 0.0f, 1.0f, 0.0f);

    // Derive frustum planes
    float4 frustumPlanes[6];
    // Sides
    frustumPlanes[0] = c4 - c1;
    frustumPlanes[1] = c4 + c1;
    frustumPlanes[2] = c4 - c2;
    frustumPlanes[3] = c4 + c2;
    // Near/far
    frustumPlanes[4] = float4(0.0f, 0.0f, 1.0f, -minTileZ);
    frustumPlanes[5] = float4(0.0f, 0.0f, -1.0f, maxTileZ);
    
    //上下左右の平面を正規化(NearとFarはすでに正規化されている)
    [unroll]
    for (uint i = 0; i < 4; ++i)
    {
        frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));
    }

    //求めた４つの平面とライトのカリングを行う
    uint totalLights = totalPointLights;
    for (uint lightIndex = groupIndex; lightIndex < totalLights; lightIndex += COMPUTE_SHADER_TILE_GROUP_SIZE)
    {
        PointLight light = pointLights[lightIndex];
                
        // Cull: point light sphere vs tile frustum
        bool inFrustum = true;
        [unroll]
        for (uint i = 0; i < 6; ++i)
        {
            //平面との距離がポイントライトの半径以下なら衝突
            float d = dot(frustumPlanes[i], float4(light.positionView, 1.0f));
            inFrustum = inFrustum && (d >= -light.attenuationEnd);
        }

        //衝突しているライトならリストに加える衝突ライトカウントもインクリメント
        [branch]
        if (inFrustum)
        {
            uint listIndex;
            InterlockedAdd(sTileNumLights, 1, listIndex);
            sTileLightIndices[listIndex] = lightIndex;
        }
    }

    //すべてのスレッドのライトカリングを待機
    GroupMemoryBarrierWithGroupSync();

    uint numLights = sTileNumLights;

    //タイルが解像度によってははみ出るのでチェック
    if (all(globalCoords < mFramebufferDimensions.xy))
    {
        if (numLights > 0)
        {
            bool perSampleShading = RequiresPerSampleShading(surfaceSamples);
            float3 lit = float3(0.0f, 0.0f, 0.0f);
            for (uint tileLightIndex = 0; tileLightIndex < numLights; ++tileLightIndex)
            {
                //ポイントライトのライティング
                PointLight light = pointLights[sTileLightIndices[tileLightIndex]];
                    //AccumulateBRDF(surfaceSamples[0], light, lit);
                lit += float3(asfloat(numLights) / 5, 0, 0);

            }

            //フレームバッファに書き込み
            WriteSample(globalCoords, 0, float4(lit, 1.0f));
        }
    }
    else
    {
      //はみ出たエリアは黒で塗りつぶしておく
        [unroll]
        for (uint sample = 0; sample < MSAA_SAMPLES; ++sample)
        {
            WriteSample(globalCoords, sample, float4(0.0f, 0.0f, 0.0f, 0.0f));
        }
    }
}