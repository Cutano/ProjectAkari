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
	float3 V = normalize(ViewPosWS.xyz - input.posWS);
	return float4(1.0f, 1.0f, 1.0f, dot(V, float3(0, 1, 0)));
}