#pragma once
#include "RenderContext.h"

namespace Akari
{
    class Visitor;
    
    class RenderPass
    {
    public:
        RenderPass() = default;
        virtual ~RenderPass() = default;

        virtual void Render(const RenderContext& context) = 0;
    protected:
        std::unique_ptr<Visitor> m_Visitor = nullptr;
    };
    
}
