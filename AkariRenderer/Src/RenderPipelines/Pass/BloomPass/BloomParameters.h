#pragma once

namespace Akari
{
    struct BloomParameters
    {
        float Threshold       = 1.0f;
        float Intensity       = 1.0f;
        float Clamp           = 65536.0f;
        float SoftKnee        = 0.5f;
        float Diffusion       = 7.0f;
        float AnamorphicRatio = 0.0f;
    };

    inline static BloomParameters g_BloomParameters;
}
