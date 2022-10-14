#pragma once
#include "RPI/RenderPass.h"
#include "SceneComponents/Visitor.h"

namespace Akari
{
    class Scene;
    class CommandList;
    class EditorCamera;
    class RenderStateObject;
    class SceneObject;
    class ShaderResourceView;

    class ForwardOpaquePass : public RenderPass
    {
    public:
        ForwardOpaquePass(
            std::shared_ptr<RenderTarget> renderTarget,
            std::shared_ptr<ShaderResourceView> skyboxSRV,
            std::shared_ptr<ShaderResourceView> skyboxIrrSRV);

        void Record(const RenderContext& context) override;
        void Execute() override;

    private:
        std::shared_ptr<RenderStateObject> m_RenderState;
        
        std::shared_ptr<ShaderResourceView> m_SkyboxSRV;
        std::shared_ptr<ShaderResourceView> m_SkyboxIrrSRV;
    };

    class ForwardOpaqueVisitor : public Visitor
    {
    public:
        ForwardOpaqueVisitor(CommandList& commandList, const RenderContext& context, RenderStateObject& state);

        void Visit(Scene& scene) override;
        void Visit(SceneObject& model) override;
        void Visit(ModelNode& modelNode) override;
        void Visit(Mesh& mesh) override;

    private:
        CommandList& m_Cmd;
        RenderStateObject& m_RenderState;
        EditorCamera& m_Camera;
    };
}
