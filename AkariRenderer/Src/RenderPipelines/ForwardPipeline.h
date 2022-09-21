#pragma once
#include "RPI/RenderPipeline.h"

namespace Akari
{
    class RenderTarget;
    
    class ForwardPipeline : public RenderPipeline
    {
    public:
        ForwardPipeline();

        void Render(const RenderContext& context) override;
        
    };
}
