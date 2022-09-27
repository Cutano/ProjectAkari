cbuffer vertexBuffer : register(b0)
{
	float4x4 ProjectionMatrix;
}

struct PS_INPUT
{
	float4 pos    : SV_POSITION;
	float3 posWS  : TEXCOORD0;
};

PS_INPUT main(float3 pos : POSITION)
{
	PS_INPUT o;
	o.pos = mul(ProjectionMatrix, float4(pos.x, pos.y, pos.z, 1.0f));
	o.posWS = pos;
	return o;
}