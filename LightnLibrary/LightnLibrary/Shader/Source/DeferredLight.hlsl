

//������2��Ō�������W��
float PhysicalAttenuation(float constantA,float linearA,float quadraticA,float lightDistance){

    return 1.0 / (constantA + linearA * lightDistance + quadraticA * lightDistance * lightDistance);
}

float PowAttenuation(float distance)
{
    return distance * distance;
}

//�X�N���[��UV�̃f�v�X�l��p���ă��[���h���W�𕜌�
float3 ReconstructWorldPositionFromDepth(Texture2D depthTex, SamplerState samLinear, float2 textureCoord, matrix inverseViewProjection)
{
	float depth = depthTex.Sample(samLinear, textureCoord).r;
	float4 projectedPosition = float4(textureCoord.xy * 2 - float2(1, 1), depth, 1.0);
    projectedPosition.y *= -1;

	float4 position = mul(projectedPosition, inverseViewProjection);
	return position.xyz / position.w;
}

float GetSquareFalloffAttenuation(float3 posToLight, float lightInvRadius)
{
    float distanceSquare = dot(posToLight, posToLight);
    float factor = distanceSquare * lightInvRadius * lightInvRadius;
    float smoothFactor = max(1.0 - factor * factor, 0.0);
    return (smoothFactor * smoothFactor) / max(distanceSquare, 1e-4);
}