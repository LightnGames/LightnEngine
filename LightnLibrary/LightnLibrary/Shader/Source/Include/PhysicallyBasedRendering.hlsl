#define PI 3.1415926535897
#define EPSILON 1e-6

//マイクロファセット分布関数
float D_GGX(float a, float dotNH) {
	float a2 = a * a;
	float dotNH2 = dotNH * dotNH;
	float d = dotNH2 * (a2 - 1.0) + 1.0;
	return a2 / (PI * d * d);
}

//フレネル
float3 F_Schlick(float3 specularColor, float3 H, float3 V) {
	return (specularColor + (1.0 - specularColor) * pow(1.0 - saturate(dot(V, H)), 5.0));
}

//幾何減衰項
float G_Smith_Schlick_GGX(float a, float dotNV, float dotNL) {
	float k = pow(a + 1, 2) * 0.5 + EPSILON;
	float gl = dotNL / (dotNL * (1.0 - k) + k);
	float gv = dotNV / (dotNV * (1.0 - k) + k);
	return gl * gv;
}

//拡散反射BRDF
float3 DiffuseBRDF(float3 diffuseColor) {
	return diffuseColor / PI;
}

//鏡面反射BRDF
float3 SpecularBRDF(float3 N, float3 V, float3 L, float3 specularColor, float roughnessFactor) {
	float dotNL = saturate(dot(N, L));
	float dotNV = saturate(dot(N, V));
	float3 H = normalize(L + V);

	float dotNH = saturate(dot(N, H));
	float dotVH = saturate(dot(V, H));
	float dotLV = saturate(dot(L, V));

	float a = roughnessFactor * roughnessFactor;

	float D = D_GGX(a, dotNH);
	float G = G_Smith_Schlick_GGX(a, dotNV, dotNH);
	float3 F = F_Schlick(specularColor, V, H);

	return (F * G * D) / (4.0 * dotNL * dotNV + EPSILON);
}

float G_Smith(float Roughness, float NoL, float NoV) {
	//Source?
	//float a = Roughness * Roughness;
	//float k = pow(0.8 + 0.5 * a, 2.0) / 2.0;

	//Source: UE4
	//float a = Roughness + 1;
	//float k = a * a / 8;

	//Source: ? - my implementation of ggx
	float a = Roughness * Roughness;
	float k = a / 2.0;

	//Source: UE4
	float G1l = NoL / (NoL * (1.0 - k) + k);
	float G1v = NoV / (NoV * (1.0 - k) + k);

	float Glvn = G1l * G1v;
	return Glvn;
}

//2bit整数を反転
float radicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

//半球上のポイントを取得
//ハンマーズリー http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float2 Hammersley(uint i, uint N) {
	return float2(i / N, radicalInverse_VdC(i));
}

//GGX項
float3 ImportanceSampleGGX(float2 Xi, float Roughness, float3 N) {

	float a = Roughness * Roughness;
	float Phi = 2 * PI * Xi.x;
	float CosTheta = sqrt((1 - Xi.y) / (1 + (a * a - 1) * Xi.y));
	float SinTheta = sqrt(1 - CosTheta * CosTheta);

	float3 H;
	H.x = SinTheta * cos(Phi);
	H.y = SinTheta * sin(Phi);
	H.z = CosTheta;


	float3 UpVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 TangentX = normalize(cross(UpVector, N));
	float3 TangentY = cross(N, TangentX);
	// Tangent to world space
	return TangentX * H.x + TangentY * H.y + N * H.z;
}

//キューブマップから環境スペキュラ色を参照
float3 PrefilterEnvMap(float Roughness, float3 R) {
	float TotalWeight = 0;

	float3 N = R;
	float3 V = R;

	float3 PrefilteredColor = 0;
	const uint NumSamples = 16;

	int maxMipLevels, width, height;
	cubeMap.GetDimensions(0, width, height, maxMipLevels);

	for (uint i = 0; i < NumSamples; i++) {

		float2 Xi = Hammersley(i, NumSamples);

		float3 H = ImportanceSampleGGX(Xi, Roughness, N);

		float3 L = 2 * dot(V, H) * H - V;

		float NoL = saturate(dot(N, L));
		if (NoL > 0) {
			PrefilteredColor += cubeMap.SampleLevel(samLinear, L, Roughness * maxMipLevels).rgb * NoL;
			TotalWeight += NoL;
		}
	}

	return PrefilteredColor / TotalWeight;
}

float3 CubemapDiffuse(float3 N) {

	float3 PrefilteredColor = 0;
	const uint NumSamples = 8;

	int maxMipLevels, width, height;
    cubeMap.GetDimensions(0, width, height, maxMipLevels);

	for (uint i = 0; i < NumSamples; i++) {

		float3 Xi = float3(Hammersley(i, NumSamples), 0);
		//Xi += N;
		Xi = lerp(N,Xi,0.5);

		PrefilteredColor += cubeMap.SampleLevel(samLinear, Xi, 0).rgb;
	}

	return PrefilteredColor / NumSamples;

}

//キューブマップから環境色を参照
float2 IntegrateBRDF(float Roughness, float NoV) {

	float3 V;
	V.x = sqrt(1.0f - NoV * NoV); // sin
	V.y = 0;
	V.z = NoV; // cos

	float A = 0;
	float B = 0;

	const uint NumSamples = 16;
	for (uint i = 0; i < NumSamples; i++) {
		float2 Xi = Hammersley(i, NumSamples);
		float3 H = ImportanceSampleGGX(Xi, Roughness, float3(0, 0, 1));
		float3 L = 2 * dot(V, H) * H - V;

		float NoL = saturate(L.z);
		float NoH = saturate(H.z);
		float VoH = saturate(dot(V, H));

		if (NoL > 0) {

			float G = G_Smith(Roughness, NoV, NoL);
			float G_Vis = G * VoH / (NoH * NoV);

			float Fc = pow(1 - VoH, 5);
			A += (1 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}

	return float2(A, B) / NumSamples;
}

//総合IBLライティングカラーを取得
float3 ApproximateSpecularIBL(float3 SpecularColor, float Roughness, float3 N, float3 V) {

	float NoV = saturate(dot(N, V));
	float3 R = normalize(2 * dot(V, N) * N - V);
	float3 PrefilteredColor = PrefilterEnvMap(Roughness, R);
	float2 EnvBRDF = IntegrateBRDF(Roughness, NoV);

	return PrefilteredColor * (SpecularColor * EnvBRDF.x + EnvBRDF.y);
}

float3 schlick(float3 f0, float product) {
	return f0 + (1.0 - f0) * pow((1.0 - product), 5.0);
}