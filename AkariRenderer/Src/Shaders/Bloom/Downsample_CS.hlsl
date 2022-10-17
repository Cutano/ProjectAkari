#include "Bloom.hlsli"

[numthreads(1, 1, 1)]
void Downsample(uint3 DTid : SV_DispatchThreadID)
{
    float2 uv = TextureTexelSize.zw * (DTid.xy + 0.5);
    
    half4 color = DownsampleBox13Tap(TEXTURE2D_PARAM(PreviousTexture, LinearClampSampler), uv, (TextureTexelSize).xy);
    OutTexture[DTid.xy] = color.rgb;
}