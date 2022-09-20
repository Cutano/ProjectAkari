#include "pch.h"

#include "PipelineStateObject.h"

#include "Device.h"

using namespace Akari;

PipelineStateObject::PipelineStateObject(Device& device, const D3D12_PIPELINE_STATE_STREAM_DESC& desc)
    : m_Device(device)
{
    auto d3d12Device = device.GetD3D12Device();

    ThrowIfFailed( d3d12Device->CreatePipelineState( &desc, IID_PPV_ARGS( &m_d3d12PipelineState ) ) );
}