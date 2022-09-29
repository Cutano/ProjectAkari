#include "pch.h"
#include "ToneMappingPass.h"
#include "ToneMappingParameters.h"

#include "RHI/Device.h"
#include "RHI/Renderer.h"
#include "RHI/RenderTarget.h"
#include "RHI/RootSignature.h"
#include "Shaders/Generated/HDRtoSDR_VS.h"
#include "Shaders/Generated/HDRtoSDR_PS.h"

namespace Akari
{
    ToneMappingPass::ToneMappingPass(std::shared_ptr<RenderTarget> renderTarget) : RenderPass(renderTarget)
    {
        CD3DX12_DESCRIPTOR_RANGE1 descriptorRange( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 );

        CD3DX12_ROOT_PARAMETER1 rootParameters[2];
        rootParameters[0].InitAsConstants( sizeof( ToneMappingParameters ) / 4, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL );
        rootParameters[1].InitAsDescriptorTable( 1, &descriptorRange, D3D12_SHADER_VISIBILITY_PIXEL );

        CD3DX12_STATIC_SAMPLER_DESC linearClampsSampler(
            0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP );

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
        rootSignatureDescription.Init_1_1( 2, rootParameters, 1, &linearClampsSampler );

        m_RootSig = Renderer::GetInstance().GetDevice()->CreateRootSignature( rootSignatureDescription.Desc_1_1 );

        CD3DX12_RASTERIZER_DESC rasterizerDesc( D3D12_DEFAULT );
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

        struct SDRPipelineStateStream
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
            CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
            CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
            CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            Rasterizer;
            CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        } sdrPipelineStateStream;

        sdrPipelineStateStream.pRootSignature        = m_RootSig->GetD3D12RootSignature().Get();
        sdrPipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        sdrPipelineStateStream.VS                    = {g_HDRtoSDR_VS, sizeof g_HDRtoSDR_VS};
        sdrPipelineStateStream.PS                    = {g_HDRtoSDR_PS, sizeof g_HDRtoSDR_PS};
        sdrPipelineStateStream.Rasterizer            = rasterizerDesc;
        sdrPipelineStateStream.RTVFormats            = renderTarget->GetRenderTargetFormats();

        m_PipelineState = Renderer::GetInstance().GetDevice()->CreatePipelineStateObject( sdrPipelineStateStream );
    }

    void ToneMappingPass::Record(const RenderContext& context)
    {
        m_Cmd = Renderer::GetInstance().GetCommandListDirect();
    }

    void ToneMappingPass::Execute()
    {
        Renderer::GetInstance().ExecuteCommandList(m_Cmd);
    }
}
