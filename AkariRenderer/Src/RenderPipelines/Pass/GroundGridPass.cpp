#include "pch.h"
#include "GroundGridPass.h"

#include "RHI/CommandList.h"
#include "RHI/Device.h"
#include "RHI/RootSignature.h"
#include "RHI/Renderer.h"
#include "RHI/RenderTarget.h"
#include "RHI/VertexBuffer.h"
#include "SceneComponents/Scene.h"
#include "SceneComponents/Camera/PerspectiveCamera.h"
#include "Shaders/Generated/GroundGrid_VS.h"
#include "Shaders/Generated/GroundGrid_PS.h"

using namespace Math;

namespace Akari
{
    GroundGridPass::GroundGridPass(std::shared_ptr<RenderTarget> renderTarget) : RenderPass(renderTarget)
    {
        m_Cmd = Renderer::GetInstance().GetCommandListDirect();
        
        // clang-format off
        constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            {
                "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
            }
        };

        constexpr int lines = 1602;
        constexpr float endpoint = (lines - 2) / 4.0f;
        static_assert((lines & 1) == 0 && (lines / 2 & 1) == 1);
        XMFLOAT2 vertices[lines * 2];
        // clang-format on

        // Generate grid vertices
        for (int i = 0; i < lines * 2; i += 4)
        {
            vertices[i + 0] = XMFLOAT2(-endpoint, static_cast<float>(static_cast<int>((-lines / 4)) + static_cast<int>((i / 4))));
            vertices[i + 1] = XMFLOAT2(endpoint, static_cast<float>(static_cast<int>((-lines / 4)) + static_cast<int>((i / 4))));
            vertices[i + 2] = XMFLOAT2(static_cast<float>(static_cast<int>((-lines / 4)) + static_cast<int>((i / 4))), -endpoint);
            vertices[i + 3] = XMFLOAT2(static_cast<float>(static_cast<int>((-lines / 4)) + static_cast<int>((i / 4))), endpoint);
        }

        m_VertexBuffer = m_Cmd->CopyVertexBuffer(lines * 2, sizeof(XMFLOAT2), vertices);
        
        constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

        CD3DX12_ROOT_PARAMETER1 rootParameters[NumRootParams];
        rootParameters[MatrixCB].InitAsConstants(
            sizeof(XMMATRIX) / 4, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
        rootSignatureDescription.Init_1_1(NumRootParams, rootParameters, 0, nullptr, rootSignatureFlags);
        
        m_RootSig = Renderer::GetInstance().GetDevice()->CreateRootSignature(rootSignatureDescription.Desc_1_1);

        D3D12_BLEND_DESC blendDesc = {};
        blendDesc.RenderTarget[0].BlendEnable = true;
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        D3D12_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        rasterizerDesc.FrontCounterClockwise = FALSE;
        rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rasterizerDesc.DepthClipEnable = true;
        rasterizerDesc.MultisampleEnable = true;
        rasterizerDesc.AntialiasedLineEnable = FALSE;
        rasterizerDesc.ForcedSampleCount = 0;
        rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable = false;
        depthStencilDesc.StencilEnable = false;

        // Setup the pipeline state.
        struct PipelineStateStream
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
            CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
            CD3DX12_PIPELINE_STATE_STREAM_VS VS;
            CD3DX12_PIPELINE_STATE_STREAM_PS PS;
            CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
            CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC SampleDesc;
            CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC BlendDesc;
            CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER RasterizerState;
            CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL DepthStencilState;
        } pipelineStateStream;

        pipelineStateStream.pRootSignature = m_RootSig->GetD3D12RootSignature().Get();
        pipelineStateStream.InputLayout = {inputLayout, _countof(inputLayout)};
        pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        pipelineStateStream.VS = {g_GroundGrid_VS, sizeof(g_GroundGrid_VS)};
        pipelineStateStream.PS = {g_GroundGrid_PS, sizeof(g_GroundGrid_PS)};
        pipelineStateStream.RTVFormats = m_RenderTarget->GetRenderTargetFormats();
        pipelineStateStream.SampleDesc = m_RenderTarget->GetSampleDesc();
        pipelineStateStream.BlendDesc = CD3DX12_BLEND_DESC(blendDesc);
        pipelineStateStream.RasterizerState = CD3DX12_RASTERIZER_DESC(rasterizerDesc);
        pipelineStateStream.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(depthStencilDesc);

        m_PipelineState = Renderer::GetInstance().GetDevice()->CreatePipelineStateObject(pipelineStateStream);

        Renderer::GetInstance().ExecuteCommandList(m_Cmd);
    }

    void GroundGridPass::Record(const RenderContext& context)
    {
        m_Cmd = Renderer::GetInstance().GetCommandListDirect();
        
        const auto cam = context.scene->GetCamera();
        const auto mvp = cam->GetViewProjMatrix();

        D3D12_VIEWPORT viewport = {};
        viewport.Width = static_cast<FLOAT>(m_RenderTarget->GetWidth());
        viewport.Height = static_cast<FLOAT>(m_RenderTarget->GetHeight());
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        
        m_Cmd->SetPipelineState(m_PipelineState);
        m_Cmd->SetGraphicsRootSignature(m_RootSig);
        m_Cmd->SetRenderTarget(*m_RenderTarget);
        m_Cmd->SetViewport(viewport);
        m_Cmd->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        m_Cmd->SetVertexBuffer(0, m_VertexBuffer);
        m_Cmd->SetGraphics32BitConstants(MatrixCB, mvp);
        
        m_Cmd->Draw(static_cast<uint32_t>(m_VertexBuffer->GetNumVertices()));
    }

    void GroundGridPass::Execute()
    {
        Renderer::GetInstance().ExecuteCommandList(m_Cmd);
    }
}
