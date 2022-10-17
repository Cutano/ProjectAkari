#ifndef SAMPLING
#define SAMPLING

// Better, temporally stable box filtering
// [Jimenez14] http://goo.gl/eomGso
// . . . . . . .
// . A . B . C .
// . . D . E . .
// . F . G . H .
// . . I . J . .
// . K . L . M .
// . . . . . . .
half4 DownsampleBox13Tap(TEXTURE2D_ARGS(tex, samplerTex), float2 uv, float2 texelSize)
{
    half4 A = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(-1.0, -1.0));
    half4 B = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(0.0, -1.0));
    half4 C = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(1.0, -1.0));
    half4 D = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(-0.5, -0.5));
    half4 E = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(0.5, -0.5));
    half4 F = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(-1.0, 0.0));
    half4 G = SAMPLE_TEXTURE2D(tex, samplerTex, uv);
    half4 H = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(1.0, 0.0));
    half4 I = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(-0.5, 0.5));
    half4 J = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(0.5, 0.5));
    half4 K = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(-1.0, 1.0));
    half4 L = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(0.0, 1.0));
    half4 M = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(1.0, 1.0));

    half2 div = (1.0 / 4.0) * half2(0.5, 0.125);

    half4 o = (D + E + I + J) * div.x;
    o += (A + B + G + F) * div.y;
    o += (B + C + H + G) * div.y;
    o += (F + G + L + K) * div.y;
    o += (G + H + M + L) * div.y;

    return o;
}

half3 DownsampleBoxPrefilter(TEXTURE2D_ARGS(tex, samplerTex), float2 uv, float2 texelSize)
{
    half3 A = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(-1.0, -1.0)).rgb;
    half3 B = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(0.0, -1.0)).rgb;
    half3 C = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(1.0, -1.0)).rgb;
    half3 D = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(-0.5, -0.5)).rgb;
    half3 E = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(0.5, -0.5)).rgb;
    half3 F = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(-1.0, 0.0)).rgb;
    half3 G = SAMPLE_TEXTURE2D(tex, samplerTex, uv).rgb;
    half3 H = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(1.0, 0.0)).rgb;
    half3 I = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(-0.5, 0.5)).rgb;
    half3 J = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(0.5, 0.5)).rgb;
    half3 K = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(-1.0, 1.0)).rgb;
    half3 L = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(0.0, 1.0)).rgb;
    half3 M = SAMPLE_TEXTURE2D(tex, samplerTex, uv + texelSize * float2(1.0, 1.0)).rgb;

    half3 Luma_A = 1.0 / (dot(A, float3(0.2126, 0.7152, 0.0722)) + 1.0);
    half3 Luma_B = 1.0 / (dot(B, float3(0.2126, 0.7152, 0.0722)) + 1.0);
    half3 Luma_C = 1.0 / (dot(C, float3(0.2126, 0.7152, 0.0722)) + 1.0);
    half3 Luma_D = 1.0 / (dot(D, float3(0.2126, 0.7152, 0.0722)) + 1.0);
    half3 Luma_E = 1.0 / (dot(E, float3(0.2126, 0.7152, 0.0722)) + 1.0);
    half3 Luma_F = 1.0 / (dot(F, float3(0.2126, 0.7152, 0.0722)) + 1.0);
    half3 Luma_G = 1.0 / (dot(G, float3(0.2126, 0.7152, 0.0722)) + 1.0);
    half3 Luma_H = 1.0 / (dot(H, float3(0.2126, 0.7152, 0.0722)) + 1.0);
    half3 Luma_I = 1.0 / (dot(I, float3(0.2126, 0.7152, 0.0722)) + 1.0);
    half3 Luma_J = 1.0 / (dot(J, float3(0.2126, 0.7152, 0.0722)) + 1.0);
    half3 Luma_K = 1.0 / (dot(K, float3(0.2126, 0.7152, 0.0722)) + 1.0);
    half3 Luma_L = 1.0 / (dot(L, float3(0.2126, 0.7152, 0.0722)) + 1.0);
    half3 Luma_M = 1.0 / (dot(M, float3(0.2126, 0.7152, 0.0722)) + 1.0);

    half2 div = (1.0 / 4.0) * half2(0.5, 0.125);

    half3 o = (D * Luma_D + E * Luma_E + I * Luma_I + J * Luma_J) * div.x;
    o += (A * Luma_A + B * Luma_B + G * Luma_G + F * Luma_F) * div.y;
    o += (B * Luma_B + C * Luma_C + H * Luma_H + G * Luma_G) * div.y;
    o += (F * Luma_F + G * Luma_G + L * Luma_L + K * Luma_K) * div.y;
    o += (G * Luma_G + H * Luma_H + M * Luma_M + L * Luma_L) * div.y;

    return o;
}

// 9-tap bilinear upsampler (tent filter)
half4 UpsampleTent(TEXTURE2D_ARGS(tex, samplerTex), float2 uv, float2 texelSize, float4 sampleScale)
{
    float4 d = texelSize.xyxy * float4(1.0, 1.0, -1.0, 0.0) * sampleScale;

    half4 s;
    s = SAMPLE_TEXTURE2D(tex, samplerTex, (uv - d.xy));
    s += SAMPLE_TEXTURE2D(tex, samplerTex, (uv - d.wy)) * 2.0;
    s += SAMPLE_TEXTURE2D(tex, samplerTex, (uv - d.zy));

    s += SAMPLE_TEXTURE2D(tex, samplerTex, (uv + d.zw)) * 2.0;
    s += SAMPLE_TEXTURE2D(tex, samplerTex, (uv)) * 4.0;
    s += SAMPLE_TEXTURE2D(tex, samplerTex, (uv + d.xw)) * 2.0;

    s += SAMPLE_TEXTURE2D(tex, samplerTex, (uv + d.zy));
    s += SAMPLE_TEXTURE2D(tex, samplerTex, (uv + d.wy)) * 2.0;
    s += SAMPLE_TEXTURE2D(tex, samplerTex, (uv + d.xy));

    return s * (1.0 / 16.0);
}

#endif
