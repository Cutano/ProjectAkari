#include "pch.h"

#include "GenerateMipsPSO.h"

#include "Device.h"
#include "RootSignature.h"

#include "Generated/GenerateMips_CS.h"

using namespace Akari;

GenerateMipsPSO::GenerateMipsPSO( Device& device )
{
    auto d3d12Device = device.GetD3D12Device();

    CD3DX12_DESCRIPTOR_RANGE1 srcMip( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0,
                                      D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE );
    CD3DX12_DESCRIPTOR_RANGE1 outMip( D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 4, 0, 0,
                                      D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE );

    CD3DX12_ROOT_PARAMETER1 rootParameters[GenerateMips::NumRootParameters];
    rootParameters[GenerateMips::GenerateMipsCB].InitAsConstants( sizeof( GenerateMipsCB ) / 4, 0 );
    rootParameters[GenerateMips::SrcMip].InitAsDescriptorTable( 1, &srcMip );
    rootParameters[GenerateMips::OutMip].InitAsDescriptorTable( 1, &outMip );

    CD3DX12_STATIC_SAMPLER_DESC linearClampSampler( 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP );

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc( GenerateMips::NumRootParameters, rootParameters, 1,
                                                             &linearClampSampler );

    m_RootSignature = device.CreateRootSignature( rootSignatureDesc.Desc_1_1 );

    // Create the PSO for GenerateMips shader.
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_CS             CS;
    } pipelineStateStream;

    pipelineStateStream.pRootSignature = m_RootSignature->GetD3D12RootSignature().Get();
    pipelineStateStream.CS             = { g_GenerateMips_CS, sizeof( g_GenerateMips_CS ) };

    m_PipelineState = device.CreatePipelineStateObject( pipelineStateStream );

    // Create some default texture UAV's to pad any unused UAV's during mip map generation.
    m_DefaultUAV = device.AllocateDescriptors( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4 );

    for ( UINT i = 0; i < 4; ++i )
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension                    = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Format                           = DXGI_FORMAT_R8G8B8A8_UNORM;
        uavDesc.Texture2D.MipSlice               = i;
        uavDesc.Texture2D.PlaneSlice             = 0;

        d3d12Device->CreateUnorderedAccessView( nullptr, nullptr, &uavDesc, m_DefaultUAV.GetDescriptorHandle( i ) );
    }
}