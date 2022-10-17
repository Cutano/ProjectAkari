#pragma once

namespace Akari
{
    struct BloomParameters
    {
        float Threshold       = 1.0f;
        float Intensity       = 1.0f;
        float Clamp           = 8.0f;
        float SoftKnee        = 0.5f;
        float Diffusion       = 7.0f;
        float AnamorphicRatio = 0.0f;
    };

    extern BloomParameters g_BloomParameters;
}
