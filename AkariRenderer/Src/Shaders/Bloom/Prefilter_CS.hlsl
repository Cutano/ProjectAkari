#include "Bloom.hlsli"

[numthreads(1, 1, 1)]
void Prefilter(uint3 DTid : SV_DispatchThreadID)
{
    float2 uv = MainTextureTexelSize.xy * DTid.xy + 0.5;
    
    half4 color = DownsampleBox13Tap(TEXTURE2D_PARAM(PreviousTexture, LinearClampSampler), uv, (MainTextureTexelSize).xy);
    color = min(PackedParams.y, color); // clamp to max
    color = QuadraticThreshold(color, Threshold.x, Threshold.yzw);
    OutTexture[DTid.xy] = color.rgb;
}