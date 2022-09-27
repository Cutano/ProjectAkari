cbuffer vertexBuffer : register(b0)
{
	float4x4 ProjectionMatrix;
}

float4 main(float2 pos : POSITION) : SV_POSITION
{
	return mul(ProjectionMatrix, float4(pos.x, 0.0f, pos.y, 1.0f));
}