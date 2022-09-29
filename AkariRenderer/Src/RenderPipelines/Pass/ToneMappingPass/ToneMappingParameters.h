#pragma once

namespace Akari
{
    enum ToneMappingMethod : uint32_t
    {
        TM_Linear,
        TM_Reinhard,
        TM_ReinhardSq,
        TM_ACESFilmic,
    };
    
    struct ToneMappingParameters
    {
        ToneMappingParameters()
        : ToneMappingMethod( TM_Reinhard )
        , Exposure( 0.0f )
        , MaxLuminance( 1.0f )
        , K( 1.0f )
        , A( 0.22f )
        , B( 0.3f )
        , C( 0.1f )
        , D( 0.2f )
        , E( 0.01f )
        , F( 0.3f )
        , LinearWhite( 11.2f )
        , Gamma( 2.2f )
        {}

        // The method to use to perform tonemapping.
        ToneMappingMethod ToneMappingMethod;
        // Exposure should be expressed as a relative exposure value (-2, -1, 0, +1, +2 )
        float Exposure;

        // The maximum luminance to use for linear tonemapping.
        float MaxLuminance;

        // Reinhard constant. Generally this is 1.0.
        float K;

        // ACES Filmic parameters
        // See: https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting/142
        float A;  // Shoulder strength
        float B;  // Linear strength
        float C;  // Linear angle
        float D;  // Toe strength
        float E;  // Toe Numerator
        float F;  // Toe denominator
        // Note E/F = Toe angle.
        float LinearWhite;
        float Gamma;
    };

    static ToneMappingParameters g_ToneMappingParameters;
}
