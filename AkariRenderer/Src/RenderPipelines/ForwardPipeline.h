#pragma once
#include "RPI/RenderPipeline.h"
#include "Pass/ForwardOpaquePass.h"
#include "Pass/GroundGridPass.h"
#include "Pass/SkyboxPass.h"
#include "Pass/ToneMappingPass/ToneMappingPass.h"

namespace Akari
{
    class RenderTarget;
    
    class ForwardPipeline : public RenderPipeline
    {
    public:
        ForwardPipeline();

        void Prepare() override;
        void Render(const RenderContext& context) override;

    private:
        std::unique_ptr<SkyboxPass> m_SkyboxPass = nullptr;
        std::unique_ptr<GroundGridPass> m_GroundGridPass = nullptr;
        std::unique_ptr<ForwardOpaquePass> m_ForwardOpaquePass = nullptr;
        std::unique_ptr<ToneMappingPass> m_ToneMappingPass = nullptr;

        std::shared_ptr<Texture> m_SkyboxPano;
        std::shared_ptr<Texture> m_SkyboxCubemap;
        std::shared_ptr<ShaderResourceView> m_SkyboxSRV;

        std::shared_ptr<Texture> m_SkyboxIrrPano;
        std::shared_ptr<Texture> m_SkyboxIrrCubemap;
        std::shared_ptr<ShaderResourceView> m_SkyboxIrrSRV;

        std::shared_ptr<Texture> m_IBLTexture;
        std::shared_ptr<ShaderResourceView> m_IBLTextureSRV;
    };
}
