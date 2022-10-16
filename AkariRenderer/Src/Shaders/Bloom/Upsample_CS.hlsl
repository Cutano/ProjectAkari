#include "Bloom.hlsli"

[numthreads(1, 1, 1)]
void Upsample(uint3 DTid : SV_DispatchThreadID)
{
    float2 uv = MainTextureTexelSize.xy * DTid.xy + 0.5;
    
    half4 bloom = UpsampleTent(TEXTURE2D_PARAM(PreviousTexture, LinearClampSampler), uv, (MainTextureTexelSize).xy, PackedParams.x);
    OutTexture[DTid.xy] = Combine(bloom, uv).rgb;
}