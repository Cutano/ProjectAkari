#pragma once
#include "RenderContext.h"

namespace Akari
{
    class Visitor;
    
    class RenderPass
    {
    public:
        RenderPass();
        virtual ~RenderPass();

        virtual void Render(const RenderContext& context);
    protected:
        std::unique_ptr<Visitor> m_Visitor;
    };
    
}
