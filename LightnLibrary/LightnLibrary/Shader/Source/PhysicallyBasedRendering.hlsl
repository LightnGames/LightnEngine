#define PI 3.1415926535897
#define EPSILON 1e-6

//}CNt@Zbgชzึ
float D_GGX(float a, float dotNH) {
	float a2 = a * a;
	float dotNH2 = dotNH * dotNH;
	float d = dotNH2 * (a2 - 1.0) + 1.0;
	return a2 / (PI * d * d);
}

//tl
float3 F_Schlick(float3 specularColor, float3 H, float3 V) {
	return (specularColor + (1.0 - specularColor) * pow(1.0 - saturate(dot(V, H)), 5.0));
}

//๔ฝธ
float G_Smith_Schlick_GGX(float a, float dotNV, float dotNL) {
	float k = pow(a + 1, 2) * 0.5 + EPSILON;
	float gl = dotNL / (dotNL * (1.0 - k) + k);
	float gv = dotNV / (dotNV * (1.0 - k) + k);
	return gl * gv;
}

//gUฝหBRDF
float3 DiffuseBRDF(float3 diffuseColor) {
	return diffuseColor / PI;
}

//พสฝหBRDF
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

//2bitฎ๐ฝ]
float radicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

//ผใฬ|Cg๐ๆพ
//n}[Y[ http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float2 Hammersley(uint i, uint N) {
	return float2(i / N, radicalInverse_VdC(i));
}

//GGX
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

float3 schlick(float3 f0, float product) {
	return f0 + (1.0 - f0) * pow((1.0 - product), 5.0);
}