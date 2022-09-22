#pragma once
#include "RPI/RenderPass.h"
#include "SceneComponents/Visitor.h"

namespace Akari
{
    class CommandList;
    class Camera;
    
    class ForwardOpaquePass : public RenderPass
    {
    public:
        ForwardOpaquePass() = default;

        void Render(const RenderContext& context) override;
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
        Camera& m_Camera;
    };
}
