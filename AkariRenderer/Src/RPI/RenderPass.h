#pragma once
#include "RenderContext.h"

namespace Akari
{
    class Visitor;
    class CommandList;
    class RootSignature;
    class RenderTarget;
    class PipelineStateObject;
    
    class RenderPass
    {
    public:
        RenderPass(std::shared_ptr<RenderTarget> renderTarget) : m_RenderTarget(renderTarget) {}
        virtual ~RenderPass() = default;

        virtual void Record(const RenderContext& context) = 0;
        virtual void Execute() = 0;
    protected:
        std::unique_ptr<Visitor> m_Visitor = nullptr;
        std::shared_ptr<CommandList> m_Cmd = nullptr;
        std::shared_ptr<RootSignature> m_RootSig = nullptr;
        std::shared_ptr<RenderTarget> m_RenderTarget = nullptr;
        std::shared_ptr<PipelineStateObject> m_PipelineState;
    };
    
}
