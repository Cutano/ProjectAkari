#ifndef BLOOM
#define BLOOM

#define TEXTURE2D_SAMPLER2D(textureName, samplerName) Texture2D textureName; SamplerState samplerName

#define TEXTURE2D(textureName) Texture2D textureName
#define SAMPLER2D(samplerName) SamplerState samplerName

#define TEXTURE2D_ARGS(textureName, samplerName) Texture2D textureName, SamplerState samplerName
#define TEXTURE2D_PARAM(textureName, samplerName) textureName, samplerName

#define SAMPLE_TEXTURE2D(textureName, samplerName, coord2) textureName.SampleLevel(samplerName, coord2, 0)
#define SAMPLE_TEXTURE2D_LOD(textureName, samplerName, coord2, lod) textureName.SampleLevel(samplerName, coord2, lod)

// -----------------------------------------------------------------------------
// Constants

#define HALF_MAX        65504.0 // (2 - 2^-10) * 2^15
#define HALF_MAX_MINUS1 65472.0 // (2 - 2^-9) * 2^15
#define EPSILON         1.0e-4
#define PI              3.14159265359
#define TWO_PI          6.28318530718
#define FOUR_PI         12.56637061436
#define INV_PI          0.31830988618
#define INV_TWO_PI      0.15915494309
#define INV_FOUR_PI     0.07957747155
#define HALF_PI         1.57079632679
#define INV_HALF_PI     0.636619772367

#define FLT_EPSILON     1.192092896e-07 // Smallest positive number, such that 1.0 + FLT_EPSILON != 1.0
#define FLT_MIN         1.175494351e-38 // Minimum representable positive floating-point number
#define FLT_MAX         3.402823466e+38 // Maximum representable floating-point number

#include "Sampling.hlsli"

cbuffer GenerateMipsCB : register(b0)
{
    float4 MainTextureTexelSize;
    float4 Intensity;
    float4 Threshold;    // x: threshold value (linear), y: threshold - knee, z: knee * 2, w: 0.25 / knee
    float4 PackedParams; // x: SampleScale, y: Clamp
}

Texture2D PreviousTexture  : register(t0);
Texture2D BloomTexture : register(t1);

RWTexture2D<float3> OutTexture : register(u0);

SamplerState LinearClampSampler : register(s0);

float Max3(float a, float b, float c)
{
    return max(max(a, b), c);
}

float4 Max3(float4 a, float4 b, float4 c)
{
    return max(max(a, b), c);
}

//
// Quadratic color thresholding
// curve = (threshold - knee, knee * 2, 0.25 / knee)
//
half4 QuadraticThreshold(half4 color, half threshold, half3 curve)
{
    // Pixel brightness
    half br = Max3(color.r, color.g, color.b);

    // Under-threshold part: quadratic curve
    half rq = clamp(br - curve.x, 0.0, curve.y);
    rq = curve.z * rq * rq;

    // Combine and apply the brightness response curve.
    color *= max(rq, br - threshold) / max(br, EPSILON);

    return color;
}

// Clamp HDR value within a safe range
half3 SafeHDR(half3 c)
{
    return min(c, HALF_MAX);
}

half4 SafeHDR(half4 c)
{
    return min(c, HALF_MAX);
}

half4 Combine(half4 bloom, float2 uv)
{
    half4 color = SAMPLE_TEXTURE2D(BloomTexture, LinearClampSampler, uv);
    return bloom + color;
}

#endif
