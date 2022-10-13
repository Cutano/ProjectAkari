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

struct VertexShaderOutput
{
	float4 PositionWS  : POSITION;
	float3 NormalWS    : NORMAL;
	float3 TangentWS   : TANGENT;
	float3 BitangentWS : BITANGENT;
	float2 TexCoord    : TEXCOORD;
	float4 Position    : SV_POSITION;
};

ConstantBuffer<LightProperties> LightPropertiesCB : register( b1 );

StructuredBuffer<DirectionalLight> DirectionalLights : register( t2 );

float4 main(VertexShaderOutput psInput) : SV_TARGET
{
	float3 mainLightDir = 0;
	if (LightPropertiesCB.NumDirectionalLights > 0)
	{
		float alpha = DirectionalLights[0].Rotation.x;
		float theta = DirectionalLights[0].Rotation.z;
		float x = sin(theta) * sin(theta);
		float z = sin(alpha) * sin(alpha);
		float y = sqrt(1.0f - x - z);

		mainLightDir = float3(x, y, z);
	}

	float NoL = dot(mainLightDir, psInput.NormalWS);
	return float4(float3(1, 1, 1) * NoL, 1.0f);
}