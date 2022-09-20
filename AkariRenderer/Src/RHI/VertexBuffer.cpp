#include "pch.h"

#include "VertexBuffer.h"

using namespace Akari;

VertexBuffer::VertexBuffer( Device& device, size_t numVertices, size_t vertexStride )
: Buffer( device, CD3DX12_RESOURCE_DESC::Buffer( numVertices * vertexStride ) )
, m_NumVertices( numVertices )
, m_VertexStride( vertexStride )
, m_VertexBufferView {}
{
    CreateVertexBufferView();
}

VertexBuffer::VertexBuffer( Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numVertices,
                            size_t vertexStride )
: Buffer( device, resource )
, m_NumVertices( numVertices )
, m_VertexStride( vertexStride )
, m_VertexBufferView {}
{
    CreateVertexBufferView();
}

VertexBuffer::~VertexBuffer() {}

void VertexBuffer::CreateVertexBufferView()
{
    m_VertexBufferView.BufferLocation = m_d3d12Resource->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes    = static_cast<UINT>( m_NumVertices * m_VertexStride );
    m_VertexBufferView.StrideInBytes  = static_cast<UINT>( m_VertexStride );
}
