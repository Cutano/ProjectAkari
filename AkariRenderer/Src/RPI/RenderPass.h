#pragma once
#include "RenderContext.h"

namespace Akari
{
    class Visitor;
    class CommandList;
    
    class RenderPass
    {
    public:
        RenderPass() = default;
        virtual ~RenderPass() = default;

        virtual void Record(const RenderContext& context) = 0;
        virtual void Execute() = 0;
    protected:
        std::unique_ptr<Visitor> m_Visitor = nullptr;
        std::shared_ptr<CommandList> m_Cmd = nullptr;
    };
    
}
