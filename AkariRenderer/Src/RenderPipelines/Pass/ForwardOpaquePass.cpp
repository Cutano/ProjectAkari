#include "pch.h"
#include "ForwardOpaquePass.h"
#include "RHI/CommandList.h"
#include "RHI/Renderer.h"
#include "SceneComponents/Scene.h"

namespace Akari
{
    void ForwardOpaquePass::Render(const RenderContext& context)
    {
        const auto& cmd = Renderer::GetInstance().GetCommandListDirect();
        ForwardOpaqueVisitor visitor(*cmd, context);
        if (context.scene)
        {
            context.scene->Accept(visitor);
        }

        Renderer::GetInstance().ExecuteCommandList(cmd);
    }

    ForwardOpaqueVisitor::ForwardOpaqueVisitor(const CommandList& commandList, const RenderContext& context) : m_Cmd(commandList), m_Camera(*context.camera)
    {
        
    }

    void ForwardOpaqueVisitor::Visit(Scene& scene)
    {
        
    }

    void ForwardOpaqueVisitor::Visit(SceneNode& sceneNode)
    {
    }

    void ForwardOpaqueVisitor::Visit(Mesh& mesh)
    {
    }
}
