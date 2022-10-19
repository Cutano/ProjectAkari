#pragma once

namespace Akari
{
    struct BloomParameters
    {
        float Threshold       = 1.0f;
        float Intensity       = 1.0f;
        float Clamp           = 65535.0f;
        float SoftKnee        = 0.0f;
        float Diffusion       = 10.0f;
        float AnamorphicRatio = 0.0f;
        bool  LowQuality      = true;
    };

    extern BloomParameters g_BloomParameters;
}
