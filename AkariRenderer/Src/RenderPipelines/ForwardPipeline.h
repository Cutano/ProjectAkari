#pragma once
#include "RPI/RenderPipeline.h"
#include "Pass/ForwardOpaquePass.h"
#include "Pass/GroundGridPass.h"

namespace Akari
{
    class RenderTarget;
    
    class ForwardPipeline : public RenderPipeline
    {
    public:
        ForwardPipeline();

        void Render(const RenderContext& context) override;

    private:
        std::unique_ptr<GroundGridPass> m_GroundGridPass = nullptr;
        std::unique_ptr<ForwardOpaquePass> m_ForwardOpaquePass = nullptr;
    };
}
