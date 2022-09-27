#include "pch.h"
#include "ForwardOpaquePass.h"
#include "RHI/CommandList.h"
#include "RHI/Renderer.h"
#include "SceneComponents/Model.h"
#include "SceneComponents/Scene.h"

namespace Akari
{
    ForwardOpaquePass::ForwardOpaquePass(std::shared_ptr<RenderTarget> renderTarget) : RenderPass(renderTarget)
    {
        
    }

    void ForwardOpaquePass::Record(const RenderContext& context)
    {
        m_Cmd = Renderer::GetInstance().GetCommandListDirect();
        // ForwardOpaqueVisitor visitor(*m_Cmd, context);
        // if (context.scene)
        // {
        //     context.scene->Accept(visitor);
        // }
    }

    void ForwardOpaquePass::Execute()
    {
        if (m_Cmd)
        {
            Renderer::GetInstance().ExecuteCommandList(m_Cmd);
        }
        else
        {
            spdlog::error("Pass executed with null command list!");
        }
    }

    ForwardOpaqueVisitor::ForwardOpaqueVisitor(const CommandList& commandList, const RenderContext& context) : m_Cmd(commandList), m_Camera(*context.scene->GetCamera())
    {
        
    }

    void ForwardOpaqueVisitor::Visit(Model& model)
    {
        
    }

    void ForwardOpaqueVisitor::Visit(ModelNode& modelNode)
    {
    }

    void ForwardOpaqueVisitor::Visit(Mesh& mesh)
    {
    }
}
