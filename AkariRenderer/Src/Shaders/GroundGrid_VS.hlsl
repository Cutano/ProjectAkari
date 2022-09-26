float4 main(float2 pos : POSITION) : SV_POSITION
{
	return float4(pos.x, 0.0f, pos.y, 1.0f);
}