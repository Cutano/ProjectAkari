#include "pch.h"
#include "BloomPass.h"

#include "BloomParameters.h"
#include "RHI/CommandList.h"
#include "RHI/Device.h"
#include "RHI/Renderer.h"
#include "RHI/RenderTarget.h"
#include "RHI/RootSignature.h"
#include "RHI/Texture.h"

#include "Shaders/Generated/Bloom/Prefilter_CS.h"
#include "Shaders/Generated/Bloom/Downsample_CS.h"
#include "Shaders/Generated/Bloom/Upsample_CS.h"
#include "Shaders/Generated/Bloom/Postfilter_CS.h"

namespace Akari
{
    BloomPass::BloomPass(const std::shared_ptr<RenderTarget>& renderTarget, const std::shared_ptr<Texture>& HDRTexture)
    : RenderPass(renderTarget), m_MainTexture(HDRTexture)
    {
        const CD3DX12_DESCRIPTOR_RANGE1 mainTexDescriptorRange(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0,
            D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

        const CD3DX12_DESCRIPTOR_RANGE1 bloomTexDescriptorRange(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0,
            D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

        const CD3DX12_DESCRIPTOR_RANGE1 outTexDescriptorRange(
            D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0,
            D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

        CD3DX12_ROOT_PARAMETER1 rootParameters[NumParams];
        rootParameters[BloomParams].InitAsConstants(4 * 4, 0, 0);
        rootParameters[PreviousTexture].InitAsDescriptorTable(1, &mainTexDescriptorRange);
        rootParameters[BloomTexture].InitAsDescriptorTable(1, &bloomTexDescriptorRange);
        rootParameters[OutTexture].InitAsDescriptorTable(1, &outTexDescriptorRange);

        const CD3DX12_STATIC_SAMPLER_DESC linearClampSampler(0,
                                                             D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                                                             D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                                             D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                                             D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

        const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc(
            NumParams, rootParameters, 1,&linearClampSampler);

        m_RootSig = Renderer::GetInstance().GetDevice()->CreateRootSignature(rootSignatureDesc.Desc_1_1);

        struct PipelineStateStream
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_CS             CS;
        };

        PipelineStateStream prefilterPSO{};
        prefilterPSO.pRootSignature = m_RootSig->GetD3D12RootSignature().Get();
        prefilterPSO.CS = {g_Prefilter_CS, sizeof g_Prefilter_CS};

        PipelineStateStream downSamplePSO{};
        downSamplePSO.pRootSignature = m_RootSig->GetD3D12RootSignature().Get();
        downSamplePSO.CS = {g_Downsample_CS, sizeof g_Downsample_CS};

        PipelineStateStream upSamplePSO{};
        upSamplePSO.pRootSignature = m_RootSig->GetD3D12RootSignature().Get();
        upSamplePSO.CS = {g_Upsample_CS, sizeof g_Upsample_CS};

        PipelineStateStream postFilterPSO{};
        postFilterPSO.pRootSignature = m_RootSig->GetD3D12RootSignature().Get();
        postFilterPSO.CS = {g_Postfilter_CS, sizeof g_Postfilter_CS};

        m_PrefilterPSO = Renderer::GetInstance().GetDevice()->CreatePipelineStateObject(prefilterPSO);
        m_DownSamplePSO = Renderer::GetInstance().GetDevice()->CreatePipelineStateObject(downSamplePSO);
        m_UpSamplePSO = Renderer::GetInstance().GetDevice()->CreatePipelineStateObject(upSamplePSO);
        m_PostFilterPSO = Renderer::GetInstance().GetDevice()->CreatePipelineStateObject(postFilterPSO);

        CD3DX12_RESOURCE_DESC desc;
        desc = CD3DX12_RESOURCE_DESC::Tex2D(m_RenderTarget->GetRenderTargetFormats().RTFormats[0], m_RenderTarget->GetWidth(), m_RenderTarget->GetHeight());
        desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        desc.Flags &= ~( D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL );

        m_OutTexture = Renderer::GetInstance().GetDevice()->CreateTexture(desc);
        m_OutTexture->SetName(L"OutTexture");

        for (int i = 0; i < k_MaxPyramidSize; i++)
        {
            m_Pyramid[i].DownSampledTexture = Renderer::GetInstance().GetDevice()->CreateTexture(desc);
            m_Pyramid[i].DownSampledTexture->SetName(std::wstring(L"DownSampledTexture ").append(std::to_wstring(i)));
            
            m_Pyramid[i].UpSampledTexture = Renderer::GetInstance().GetDevice()->CreateTexture(desc);
            m_Pyramid[i].UpSampledTexture->SetName(std::wstring(L"UpSampledTexture ").append(std::to_wstring(i)));
        }
    }

    void BloomPass::Record(const RenderContext& context)
    {
        m_Cmd = Renderer::GetInstance().GetCommandListDirect();
        m_Cmd->GetD3D12CommandList()->SetName(L"BloomPass CommandList");

        auto rtWith = m_RenderTarget->GetWidth();
        auto rtHeight = m_RenderTarget->GetHeight();

        // Negative anamorphic ratio values distort vertically - positive is horizontal
        float ratio = glm::clamp(g_BloomParameters.AnamorphicRatio, -1.0f, 1.0f);
        float rw = ratio < 0 ? -ratio : 0.0f;
        float rh = ratio > 0 ? ratio : 0.0f;

        // Do bloom on a half-res buffer, full-res doesn't bring much and kills performances on
        // fillrate limited platforms
        int tw = static_cast<int>(rtWith / (2.0f - rw));
        int th = static_cast<int>(rtHeight / (2.0f - rh));
        int tw_stereo = tw;

        // Determine the iteration count
        float s = static_cast<float>(glm::max(tw, th));
        float logs = glm::log(s, 2.0f) + glm::min(g_BloomParameters.Diffusion, 10.0f) - 10.0f;
        int logs_i = static_cast<int>(logs);
        int iterations = glm::clamp(logs_i, 1, k_MaxPyramidSize);
        float sampleScale = 0.5f + logs - static_cast<float>(logs_i);
        m_Params.Params.x = sampleScale;

        // Prefiltering parameters
        float lthresh = g_BloomParameters.Threshold;
        float knee = lthresh * g_BloomParameters.SoftKnee + 1e-5f;
        auto threshold = glm::vec4(lthresh, lthresh - knee, knee * 2.0f, 0.25f / knee);
        m_Params.Threshold = threshold;
        float lclamp = g_BloomParameters.Clamp;
        m_Params.Params.y = lclamp;

        m_Cmd->TransitionBarrier(m_MainTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        // DownSample
        std::shared_ptr<Texture> lastDown = m_MainTexture;
        for (int i = 0; i < iterations; i++)
        {
            auto pass = i == 0
                ? m_PrefilterPSO
                : m_DownSamplePSO;

            if (m_Pyramid[i].DownSampledTexture->GetD3D12ResourceDesc().Width != tw_stereo || m_Pyramid[i].DownSampledTexture->GetD3D12ResourceDesc().Height != th)
            {
                m_Pyramid[i].DownSampledTexture->Resize(tw_stereo, th);
            }

            m_Pyramid[i].Width = tw_stereo;
            m_Pyramid[i].Height = th;

            m_Params.TextureTexelSize = i == 0
                ? glm::vec4(1.0f / rtWith, 1.0f / rtHeight, 1.0f / tw_stereo, 1.0f / th)
                : glm::vec4(1.0f / m_Pyramid[i - 1].Width, 1.0f / m_Pyramid[i - 1].Height, 1.0f / tw_stereo, 1.0f / th);

            m_Cmd->SetPipelineState(pass);
            m_Cmd->SetComputeRootSignature(m_RootSig);

            m_Cmd->SetCompute32BitConstants(BloomParams, m_Params);
            m_Cmd->SetShaderResourceView(PreviousTexture, 0, lastDown, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            m_Cmd->SetShaderResourceView(BloomTexture, 0, lastDown, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE); // unused
            m_Cmd->SetUnorderedAccessView(OutTexture, 0, m_Pyramid[i].DownSampledTexture, 0);

            m_Cmd->Dispatch(tw_stereo, th);

            lastDown = m_Pyramid[i].DownSampledTexture;
            tw_stereo = tw_stereo / 2;
            tw_stereo = glm::max(tw_stereo, 1);
            th = glm::max(th / 2, 1);
        }

        // Upsample
        std::shared_ptr<Texture> lastUp = m_Pyramid[iterations - 1].DownSampledTexture;
        for (int i = iterations - 2; i >= 0; i--)
        {
            if (m_Pyramid[i].UpSampledTexture->GetD3D12ResourceDesc().Width != m_Pyramid[i].Width || m_Pyramid[i].UpSampledTexture->GetD3D12ResourceDesc().Height != m_Pyramid[i].Height)
            {
                m_Pyramid[i].UpSampledTexture->Resize(m_Pyramid[i].Width, m_Pyramid[i].Height);
            }

            m_Params.TextureTexelSize = glm::vec4(1.0f / m_Pyramid[i + 1].Width, 1.0f / m_Pyramid[i + 1].Height, 1.0f / m_Pyramid[i].Width, 1.0f / m_Pyramid[i].Height);
            
            m_Cmd->SetPipelineState(m_UpSamplePSO);
            m_Cmd->SetComputeRootSignature(m_RootSig);

            m_Cmd->SetCompute32BitConstants(BloomParams, m_Params);
            m_Cmd->SetShaderResourceView(PreviousTexture, 0, lastUp, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            m_Cmd->SetShaderResourceView(BloomTexture, 0, m_Pyramid[i].DownSampledTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            m_Cmd->SetUnorderedAccessView(OutTexture, 0, m_Pyramid[i].UpSampledTexture, 0);

            m_Cmd->Dispatch(m_Pyramid[i].Width, m_Pyramid[i].Height);

            lastUp = m_Pyramid[i].UpSampledTexture;
        }

        // PostFilter
        {
            if (m_OutTexture->GetD3D12ResourceDesc().Width != rtWith || m_OutTexture->GetD3D12ResourceDesc().Height != rtHeight)
            {
                m_OutTexture->Resize(rtWith, rtHeight);
            }
            
            m_Params.TextureTexelSize = glm::vec4(1.0f / m_Pyramid[0].Width, 1.0f / m_Pyramid[0].Height, 1.0f / rtWith, 1.0f / rtHeight);
            m_Params.Intensity = glm::vec4(g_BloomParameters.Intensity);
            
            m_Cmd->SetPipelineState(m_PostFilterPSO);
            m_Cmd->SetComputeRootSignature(m_RootSig);

            m_Cmd->SetCompute32BitConstants(BloomParams, m_Params);
            m_Cmd->SetShaderResourceView(PreviousTexture, 0, m_Pyramid[0].UpSampledTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            m_Cmd->SetShaderResourceView(BloomTexture, 0, m_MainTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            m_Cmd->SetUnorderedAccessView(OutTexture, 0, m_OutTexture, 0);

            m_Cmd->Dispatch(rtWith, rtHeight);

            m_Cmd->CopyResource(m_MainTexture, m_OutTexture);
        }
    }

    void BloomPass::Execute()
    {
        Renderer::GetInstance().ExecuteCommandList(m_Cmd);
    }
}
