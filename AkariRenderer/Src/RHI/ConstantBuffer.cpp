#include "pch.h"

#include "ConstantBuffer.h"
#include "Device.h"

using namespace Akari;

ConstantBuffer::ConstantBuffer( Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource )
: Buffer( device, resource )
{
    m_SizeInBytes = GetD3D12ResourceDesc().Width;
}

ConstantBuffer::~ConstantBuffer() {}
