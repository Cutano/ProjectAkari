#include "pch.h"

#include "StructuredBuffer.h"

#include "Device.h"
#include "ResourceStateTracker.h"
#include "d3dx12.h"

using namespace Akari;

StructuredBuffer::StructuredBuffer( Device& device, size_t numElements, size_t elementSize )
: Buffer( device,
          CD3DX12_RESOURCE_DESC::Buffer( numElements * elementSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS ) )
, m_NumElements( numElements )
, m_ElementSize( elementSize )
{
    m_CounterBuffer = m_Device.CreateByteAddressBuffer( 4 );
}

StructuredBuffer::StructuredBuffer( Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numElements,
                                    size_t elementSize )
: Buffer( device, resource )
, m_NumElements( numElements )
, m_ElementSize( elementSize )
{
    m_CounterBuffer = m_Device.CreateByteAddressBuffer( 4 );
}
