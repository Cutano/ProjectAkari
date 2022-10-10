#pragma once
#include "RPI/RenderPass.h"
#include "SceneComponents/Visitor.h"

namespace Akari
{
    class CommandList;
    class PerspectiveCamera;
    class Model;
    class Texture;
    class ShaderResourceView;
    class SceneObject;

    class SkyboxPass : public RenderPass
    {
    public:
        SkyboxPass(std::shared_ptr<RenderTarget> renderTarget);

        void Record(const RenderContext& context) override;
        void Execute() override;

    private:
        enum RootParams
        {
            // cbuffer vertexBuffer : register(b0)
            MatrixCB,
            // cbuffer dirBuffer : register(b1)
            TextureCube,
            NumRootParams
        };
        
        std::shared_ptr<Model> m_Skybox;
        std::shared_ptr<Texture> m_SkyboxPano;
        std::shared_ptr<Texture> m_SkyboxCubemap;
        std::shared_ptr<ShaderResourceView> m_SkyboxSRV;
    };

    class SkyboxVisitor : public Visitor
    {
    public:
        SkyboxVisitor(CommandList& commandList);

        void Visit(Scene& scene) override;
        void Visit(SceneObject& model) override;
        void Visit(ModelNode& modelNode) override;
        void Visit(Mesh& mesh) override;

    private:
        CommandList& m_Cmd;
    };
}
