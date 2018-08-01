

//距離の2乗で減衰する係数
float PhysicalAttenuation(float constantA,float linearA,float quadraticA,float lightDistance){

return 1.0 / (constantA + linearA * lightDistance
			+ quadraticA * lightDistance * lightDistance);
}

//スクリーンUVのデプス値を用いてワールド座標を復元
float3 ReconstructWorldPositionFromDepth(Texture2D depthTex, SamplerState samLinear, float2 textureCoord, matrix inverseViewProjection)
{
	float depth = depthTex.Sample(samLinear, textureCoord).r;
	float4 projectedPosition = float4(textureCoord.xy * 2 - float2(1, 1), depth, 1.0);

	float4 position = mul(projectedPosition, inverseViewProjection);
	position.y*=-1;
	return position.xyz / position.w;
}