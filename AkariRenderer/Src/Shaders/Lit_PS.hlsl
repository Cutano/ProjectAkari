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

ConstantBuffer<LightProperties> LightPropertiesCB : register( b1 );

StructuredBuffer<DirectionalLight> DirectionalLights : register( t2 );

float4 main() : SV_TARGET
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
	return float4(mainLightDir, 1.0f);
}