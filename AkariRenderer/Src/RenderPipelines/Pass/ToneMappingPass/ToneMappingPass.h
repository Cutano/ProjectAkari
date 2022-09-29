#pragma once
#include "RPI/RenderPass.h"
#include "SceneComponents/Visitor.h"

namespace Akari
{
    class ToneMappingPass : RenderPass
    {
    public:
        ToneMappingPass(std::shared_ptr<RenderTarget> renderTarget);

        void Record(const RenderContext& context) override;
        void Execute() override;
    };
}
