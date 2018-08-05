

//������2��Ō�������W��
float PhysicalAttenuation(float constantA,float linearA,float quadraticA,float lightDistance){

    return 1.0 / (constantA + linearA * lightDistance+ quadraticA * lightDistance * lightDistance);
}

//�X�N���[��UV�̃f�v�X�l��p���ă��[���h���W�𕜌�
float3 ReconstructWorldPositionFromDepth(Texture2D depthTex, SamplerState samLinear, float2 textureCoord, matrix inverseViewProjection)
{
	float depth = depthTex.Sample(samLinear, textureCoord).r;
	float4 projectedPosition = float4(textureCoord.xy * 2 - float2(1, 1), depth, 1.0);

	float4 position = mul(projectedPosition, inverseViewProjection);
	position.y*=-1;
	return position.xyz / position.w;
}