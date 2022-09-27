#pragma once
#include "RPI/RenderPass.h"
#include "SceneComponents/Visitor.h"

namespace Akari
{
    class CommandList;
    class PerspectiveCamera;
    
    class ForwardOpaquePass : public RenderPass
    {
    public:
        ForwardOpaquePass(std::shared_ptr<RenderTarget> renderTarget);

        void Record(const RenderContext& context) override;
        void Execute() override;
    };

    class ForwardOpaqueVisitor : public Visitor
    {
    public:
        ForwardOpaqueVisitor(const CommandList& commandList, const RenderContext& context);

        void Visit(Model& model) override;
        void Visit(ModelNode& modelNode) override;
        void Visit(Mesh& mesh) override;

    private:
        const CommandList& m_Cmd;
        PerspectiveCamera& m_Camera;
    };
}
