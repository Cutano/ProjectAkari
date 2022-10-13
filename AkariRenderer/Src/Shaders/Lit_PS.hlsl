struct Matrices
{
	matrix ModelMatrix;
	matrix ViewMatrix;
	matrix ProjectionMatrix;
	matrix MVP;
	matrix InverseViewMatrix;
};

struct LightProperties
{
	uint NumPointLights;
	uint NumSpotLights;
	uint NumDirectionalLights;
};

struct DirectionalLight
{
	float3 Translation;
	float3 Rotation;
	float3 Scale;
	bool CastShadows;
	bool SoftShadows;
	float3 Radiance;
	float Intensity;
	float LightSize; // For PCSS
	float ShadowAmount;
};

struct MaterialProperties
{
	float4 BaseColor;
	//------------------------------------ ( 16 bytes )
	float4 Emissive;
	//------------------------------------ ( 16 bytes )
	float Opacity;                       // If Opacity < 1, then the material is transparent.
	float Roughness;
	float Metallic;             
	float NormalScale;

	//------------------------------------ ( 16 bytes )
	uint HasBaseColorTexture;
	uint HasMetallicTexture;
	uint HasRoughnessTexture;
	uint HasEmissiveTexture;
	//------------------------------------ ( 16 bytes )
	uint HasOcclusionTexture;
	uint HasNormalTexture;
	uint HasBumpTexture;
	uint HasOpacityTexture;
	//------------------------------------ ( 16 bytes )
	// Total:                              ( 16 * 8 = 128 bytes )
};

struct VertexShaderOutput
{
	float4 PositionWS  : POSITION;
	float3 NormalWS    : NORMAL;
	float3 TangentWS   : TANGENT;
	float3 BitangentWS : BITANGENT;
	float2 TexCoord    : TEXCOORD;
	float4 Position    : SV_POSITION;
};

ConstantBuffer<Matrices> MatCB : register(b0, space0);
ConstantBuffer<MaterialProperties> MaterialCB : register(b0, space1);
ConstantBuffer<LightProperties> LightPropertiesCB : register(b1);

StructuredBuffer<DirectionalLight> DirectionalLights : register(t2);

static const float PI = 3.141592653589793;

float3 Fresnel(float3 f0, float3 viewDir, float3 halfWay)
{
	return f0 + (1.0f - f0) * (1.0f - dot(viewDir, halfWay));
}

float GGX(float3 m, float3 normalWS, float alpha)
{
	return alpha * alpha / (PI * pow(dot(normalWS, m) * dot(normalWS, m) * (alpha * alpha - 1) + 1, 2));
}

float G1(float3 v, float3 normalWS, float k)
{
	return dot(normalWS, v) / (dot(normalWS, v) * (1.0f - k) + k);
}

float SchlickGGX(float alpha, float3 normalWS, float3 viewDir, float3 lightDir)
{
	float k = alpha * 0.5f;
	return G1(lightDir, normalWS, k) * G1(viewDir, normalWS, k);
}

float3 CookTorranceBRDF(float3 viewDir, float3 lightDir, float3 normalWS, float3 f0, float roughness)
{
	float alpha = (roughness + 1.0f) * (roughness + 1.0f) * 0.25f;
	float3 h = (lightDir + viewDir) * 0.5f;
	return GGX(h, normalWS, alpha) * Fresnel(f0, viewDir, h) * SchlickGGX(alpha, normalWS, viewDir, lightDir);
}

float4 main(VertexShaderOutput psInput) : SV_TARGET
{
	float3 viewPosWS = MatCB.InverseViewMatrix._14_24_34;
	float3 viewDir = normalize(viewPosWS - psInput.PositionWS.xyz);
	float3 mainLightDir = 0;
	if (LightPropertiesCB.NumDirectionalLights > 0)
	{
		float pitch = DirectionalLights[0].Rotation.x;
		float roll  = DirectionalLights[0].Rotation.y;
		float yaw   = DirectionalLights[0].Rotation.z;
		// float x = -cos(roll) * sin(yaw) - sin(roll) * sin(pitch) * cos(yaw);
		// float y = cos(pitch) * cos(yaw);
		// float z = -sin(roll) * sin(yaw) + cos(roll) * sin(pitch) * cos(yaw);
		float x = -sin(yaw);
		float y = cos(pitch) * cos(yaw);
		float z = sin(pitch) * cos(yaw);

		mainLightDir = normalize(float3(x, y, z));
	}

	float NoL = saturate(dot(mainLightDir, psInput.NormalWS));
	float3 col = CookTorranceBRDF(viewDir, mainLightDir, psInput.NormalWS, float3(0.4f,0.4f,0.4f), MaterialCB.Roughness);
	return float4(col, 1);
}