#include "pch.h"
#include "RenderStateObject.h"

#include "RHI/CommandList.h"

namespace Akari
{
    RenderStateObject::RenderStateObject(std::shared_ptr<Device> device) : m_Device(device)
    {
    }

    RenderStateObject::~RenderStateObject()
    {
    }

    void RenderStateObject::SetModelMatrix(glm::mat4 modelMat)
    {
        m_MVP.Model = modelMat;
    }

    void RenderStateObject::SetViewMatrix(glm::mat4 viewMat)
    {
        m_MVP.View = viewMat;
    }

    void RenderStateObject::SetProjMatrix(glm::mat4 projMat)
    {
        m_MVP.Proj = projMat;
    }

    void RenderStateObject::SetMaterial(const std::shared_ptr<Material>& mat)
    {
        m_Material = mat;
    }

    void RenderStateObject::Apply(CommandList& cmd)
    {
        cmd.SetPipelineState(m_PipelineStateObject);
        cmd.SetGraphicsRootSignature(m_RootSig);
    }

    void RenderStateObject::BindTexture(CommandList& cmd, uint32_t offset, const std::shared_ptr<Texture>& tex) const
    {
        if ( tex )
        {
            cmd.SetShaderResourceView(Textures, offset, tex,
                                      D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );
        }
        else
        {
            cmd.SetShaderResourceView(Textures, offset, m_DefaultSRV,
                                      D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );
        }
    }
}
