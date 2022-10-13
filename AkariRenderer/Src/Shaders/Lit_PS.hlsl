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

float3 fresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
	float a      = roughness * roughness;
	float a2     = a * a;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom   = a2;
	float denom = NdotH2 * (a2 - 1.0) + 1.0;
	denom = PI * denom * denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = roughness + 1.0;
	float k = r * r / 8.0;

	float nom   = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float IBLGeometrySchlickGGX(float NdotV, float roughness)
{
	float k = roughness * roughness / 2.0;

	float nom   = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

float IBLGeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = IBLGeometrySchlickGGX(NdotV, roughness);
	float ggx1 = IBLGeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

float3 DirectPBRLighting(float3 baseColor, float3 V, float3 L, float3 N, float3 F0, float roughness, float metallic)
{
	// Cook-Torrance BRDF
	float3 H = normalize(V + L);
	
	float NDF = DistributionGGX(N, H, roughness);        
	float G   = GeometrySmith(N, V, L, roughness);      
	float3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

	float3 nominator    = NDF * G * F;
	float  denominator  = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
	float3 specular = saturate(nominator / denominator);

	float3 kS = F;
	float3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;

	float NoL = saturate(dot(N, L));

	return (kD * baseColor / PI + specular) * NoL;
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
	
	float3 col = DirectPBRLighting(MaterialCB.BaseColor.xyz, viewDir, mainLightDir, psInput.NormalWS, 0.04f, MaterialCB.Roughness, MaterialCB.Metallic);
	return float4(col, 1);
}