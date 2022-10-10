#pragma once
#include "RPI/RenderPass.h"
#include "SceneComponents/Visitor.h"

namespace Akari
{
    class CommandList;
    class EditorCamera;
    class RenderStateObject;
    
    class ForwardOpaquePass : public RenderPass
    {
    public:
        ForwardOpaquePass(std::shared_ptr<RenderTarget> renderTarget);

        void Record(const RenderContext& context) override;
        void Execute() override;

    private:
        std::shared_ptr<RenderStateObject> m_RenderState;
    };

    class ForwardOpaqueVisitor : public Visitor
    {
    public:
        ForwardOpaqueVisitor(const CommandList& commandList, const RenderContext& context, RenderStateObject& state);

        void Visit(Model& model) override;
        void Visit(ModelNode& modelNode) override;
        void Visit(Mesh& mesh) override;

    private:
        const CommandList& m_Cmd;
        RenderStateObject& m_RenderState;
        EditorCamera& m_Camera;
    };
}
