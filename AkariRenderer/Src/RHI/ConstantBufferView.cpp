#include "pch.h"

#include "ConstantBufferView.h"

#include "ConstantBuffer.h"
#include "Device.h"

using namespace Akari;

ConstantBufferView::ConstantBufferView( Device& device, const std::shared_ptr<ConstantBuffer>& constantBuffer,
                                        size_t offset )
: m_Device( device )
, m_ConstantBuffer( constantBuffer )
{
    assert( constantBuffer );

    auto d3d12Device   = m_Device.GetD3D12Device();
    auto d3d12Resource = m_ConstantBuffer->GetD3D12Resource();

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv;
    cbv.BufferLocation = d3d12Resource->GetGPUVirtualAddress() + offset;
    cbv.SizeInBytes =
        Math::AlignUp( static_cast<UINT>(m_ConstantBuffer->GetSizeInBytes()),
                       D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT );  // Constant buffers must be aligned for
                                                                          // hardware requirements.

    m_Descriptor = device.AllocateDescriptors( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

    d3d12Device->CreateConstantBufferView( &cbv, m_Descriptor.GetDescriptorHandle() );
}
