#pragma once
#include "RPI/RenderPass.h"
#include "SceneComponents/Visitor.h"

namespace Akari
{
    class Texture;
    
    class ToneMappingPass : RenderPass
    {
    public:
        ToneMappingPass(std::shared_ptr<RenderTarget> renderTarget, std::shared_ptr<Texture> hdrTex);

        void Record(const RenderContext& context) override;
        void Execute() override;

    private:
        std::shared_ptr<Texture> m_HDRTex;
    };
}
