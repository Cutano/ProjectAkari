#pragma once

namespace Akari
{
    struct BloomParameters
    {
        float Threshold       = 1.0f;
        float Intensity       = 0.2f;
        float Clamp           = 65535.0f;
        float SoftKnee        = 0.0f;
        float Diffusion       = 8.0f;
        float AnamorphicRatio = 0.0f;
        bool  LowQuality      = false;
    };

    extern BloomParameters g_BloomParameters;
}
