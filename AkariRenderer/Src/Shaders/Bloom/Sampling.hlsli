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

// 9-tap bilinear upsampler (tent filter)
half4
UpsampleTent(TEXTURE2D_ARGS(tex, samplerTex), float2 uv, float2 texelSize, float4 sampleScale)
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