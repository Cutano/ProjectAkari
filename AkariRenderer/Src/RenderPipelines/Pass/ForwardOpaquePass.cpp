#include "pch.h"
#include "ForwardOpaquePass.h"
#include "RHI/CommandList.h"
#include "RHI/Renderer.h"
#include "RHI/RenderTarget.h"
#include "RPI/RenderStateObject.h"
#include "SceneComponents/Mesh.h"
#include "SceneComponents/Light.h"
#include "SceneComponents/ModelNode.h"
#include "SceneComponents/Scene.h"
#include "SceneComponents/SceneObject.h"
#include "SceneComponents/Camera/EditorCamera.h"

#include "Shaders/Generated/Lit_VS.h"
#include "Shaders/Generated/Lit_PS.h"

namespace Akari
{
    ForwardOpaquePass::ForwardOpaquePass(
        std::shared_ptr<RenderTarget> renderTarget,
        std::shared_ptr<ShaderResourceView> skyboxSRV,
        std::shared_ptr<ShaderResourceView> skyboxIrrSRV,
        std::shared_ptr<ShaderResourceView> IBLTextureSRV)
    : RenderPass(renderTarget), m_SkyboxSRV(skyboxSRV), m_SkyboxIrrSRV(skyboxIrrSRV), m_IBLTextureSRV(IBLTextureSRV)
    {
        m_RenderState = std::make_shared<RenderStateObject>(Renderer::GetInstance().GetDevice());
        m_RenderState->SetRenderTarget(renderTarget);
        m_RenderState->SetShader(g_Lit_VS, sizeof g_Lit_VS, g_Lit_PS, sizeof g_Lit_PS);
        m_RenderState->SetCubeMaps(m_SkyboxSRV, m_SkyboxIrrSRV);
        m_RenderState->SetLUTs(m_IBLTextureSRV);
    }

    void ForwardOpaquePass::Record(const RenderContext& context)
    {
        m_Cmd = Renderer::GetInstance().GetCommandListDirect();

        m_Cmd->SetRenderTarget(*m_RenderTarget);
        m_Cmd->SetViewport(m_RenderTarget->GetViewport());
        m_Cmd->SetScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX));

        ForwardOpaqueVisitor visitor(*m_Cmd, context, *m_RenderState);
        if (context.scene)
        {
            context.scene->Accept(visitor);
        }
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

    ForwardOpaqueVisitor::ForwardOpaqueVisitor(CommandList& commandList, const RenderContext& context, RenderStateObject& state)
    : m_Cmd(commandList), m_RenderState(state), m_Camera(*context.scene->GetCamera())
    {
        
    }

    void ForwardOpaqueVisitor::Visit(Scene& scene)
    {
        const auto& dirLights = scene.GetAllSceneObjectsWith<DirectionalLightComponent>();
        std::vector<DirectionalLight> dirLightComps;

        for (const auto& dirLight : dirLights)
        {
            SceneObject obj(dirLight, &scene);
            DirectionalLight light{obj.GetComponent<TransformComponent>(), obj.GetComponent<DirectionalLightComponent>()};
            dirLightComps.push_back(light);
        }

        m_RenderState.SetDirectionalLights(dirLightComps);
        m_RenderState.SetViewMatrix(m_Camera.GetViewMatrix());
        m_RenderState.SetProjMatrix(m_Camera.GetProjectionMatrix());
    }

    void ForwardOpaqueVisitor::Visit(SceneObject& model)
    {
        const auto& trans = model.GetComponent<TransformComponent>();
        m_RenderState.SetModelMatrix(trans.GetTransform());
    }

    void ForwardOpaqueVisitor::Visit(ModelNode& modelNode)
    {
        // m_RenderState.SetModelMatrix(modelNode.GetWorldTransform());
    }

    void ForwardOpaqueVisitor::Visit(Mesh& mesh)
    {
        const auto material = mesh.GetMaterial();
        m_RenderState.SetMaterial(material);
        m_RenderState.Apply(m_Cmd);
        mesh.Draw(m_Cmd);
    }
}
