#pragma once
#include "RPI/RenderPass.h"
#include "SceneComponents/Visitor.h"

// Procedure: Prefilter -> DownSample -> UpSample
//
// Example: Texture Filtered[N], N = 7
// RT -> Filtered[0] -> Filtered[1] -> Filtered[2] -> Filtered[3]    (Prefilter & DownSampling) x 4
//           ↓              ↓              ↓              ↓
// RT <= Filtered[6] <- Filtered[5] <- Filtered[4]  <-----|          UpSampling x 3
//
// Example: iterations = 4
// RT -> Filtered[0] -> Filtered[1] -> Filtered[2] -> Filtered[3]    (Prefilter & DownSampling) x 4
//           ↓              ↓              ↓              ↓
// RT <= Filtered[0] <- Filtered[1] <- Filtered[2]  <-----|          UpSampling x 3
//
// Prefilter[0]:
//     Purpose: Clamping and DownSampling
//     Input:   Render Texture
//     Output:  Filtered Texture[0]
//
// DownSample[i]:
//     Purpose: DownSampling
//     Input:   Filtered Texture[i - 1]
//     Output:  Filtered Texture[i]
//
// UpSample[i]:
//     Purpose: UpSample and Accumulate
//     Input:   Filtered Texture[i - 1] & Filtered Texture[N - 1 - i]
//     Output:  Filtered Texture[i]

namespace Akari
{
    class Texture;
    class ShaderResourceView;
    
    class BloomPass : RenderPass
    {
    public:
        BloomPass(const std::shared_ptr<RenderTarget>& renderTarget, const std::shared_ptr<Texture>& HDRTexture);
        
        void Record(const RenderContext& context) override;
        void Execute() override;

    private:
        enum RootParams
        {
            BloomParams,  // float4 InputTextureTexelSize
                          // float4 Intensity
                          // float4 Threshold x: threshold value (linear), y: threshold - knee, z: knee * 2, w: 0.25 / knee
                          // float4 x: SampleScale, y: Clamp
            
            PreviousTexture,  // Texture2D PreviousTexture  : register(t0);
            BloomTexture,     // Texture2D BloomTexture : register(t1);
            OutTexture,       // RWTexture2D<float3> OutTexture : register(u0);
            NumParams
        };

        struct BloomParam
        {
            glm::vec4 TextureTexelSize; // xy: OutputSize zw: InputSize
            glm::vec4 Intensity;
            glm::vec4 Threshold;
            glm::vec4 Params;
        };

        struct Level
        {
            int Width;
            int Height;
            std::shared_ptr<Texture> DownSampledTexture;
            std::shared_ptr<Texture> UpSampledTexture;
        };

        inline static constexpr int k_MaxPyramidSize = 16;
        Level m_Pyramid[k_MaxPyramidSize];

        BloomParam m_Params;
        
        std::shared_ptr<Texture> m_MainTexture;
        std::shared_ptr<Texture> m_PreviousTexture;
        std::shared_ptr<Texture> m_BloomTexture;
        std::shared_ptr<Texture> m_OutTexture;

        std::shared_ptr<PipelineStateObject> m_PrefilterPSO;
        std::shared_ptr<PipelineStateObject> m_DownSamplePSO;
        std::shared_ptr<PipelineStateObject> m_UpSamplePSO;
    };
    
}
