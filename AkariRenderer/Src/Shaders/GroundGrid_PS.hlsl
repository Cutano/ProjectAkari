cbuffer dirBuffer : register(b1)
{
	float4 ViewPosWS;
}

struct PS_INPUT
{
	float4 pos    : SV_POSITION;
	float3 posWS  : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	// float3 clampedYViewPos = float3(ViewPosWS.x, clamp(ViewPosWS.y, -20.0f, 20.0f), ViewPosWS.z);
	float3 V = normalize(ViewPosWS.xyz - input.posWS);
	return float4(0.8f, 0.8f, 0.8f, clamp(pow(dot(V, float3(0, 1, 0)), 1.4f) * pow(1.1f, -ViewPosWS.y), -0.7f, 0.7f));
}