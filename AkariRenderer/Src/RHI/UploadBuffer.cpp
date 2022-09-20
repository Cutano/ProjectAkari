#include "pch.h"

#include "UploadBuffer.h"

#include "Device.h"

using namespace Akari;

UploadBuffer::UploadBuffer( Device& device, size_t pageSize )
: m_Device( device )
, m_PageSize( pageSize )
{}

UploadBuffer::~UploadBuffer() {}

UploadBuffer::Allocation UploadBuffer::Allocate( size_t sizeInBytes, size_t alignment )
{
    if ( sizeInBytes > m_PageSize )
    {
        throw std::bad_alloc();
    }

    // If there is no current page, or the requested allocation exceeds the
    // remaining space in the current page, request a new page.
    if ( !m_CurrentPage || !m_CurrentPage->HasSpace( sizeInBytes, alignment ) )
    {
        m_CurrentPage = RequestPage();
    }

    return m_CurrentPage->Allocate( sizeInBytes, alignment );
}

std::shared_ptr<UploadBuffer::Page> UploadBuffer::RequestPage()
{
    std::shared_ptr<Page> page;

    if ( !m_AvailablePages.empty() )
    {
        page = m_AvailablePages.front();
        m_AvailablePages.pop_front();
    }
    else
    {
        page = std::make_shared<Page>( m_Device, m_PageSize );
        m_PagePool.push_back( page );
    }

    return page;
}

void UploadBuffer::Reset()
{
    m_CurrentPage = nullptr;
    // Reset all available pages.
    m_AvailablePages = m_PagePool;

    for ( auto page: m_AvailablePages )
    {
        // Reset the page for new allocations.
        page->Reset();
    }
}

UploadBuffer::Page::Page( Device& device, size_t sizeInBytes )
: m_Device( device )
, m_PageSize( sizeInBytes )
, m_Offset( 0 )
, m_CPUPtr( nullptr )
, m_GPUPtr( D3D12_GPU_VIRTUAL_ADDRESS( 0 ) )
{
    auto d3d12Device = m_Device.GetD3D12Device();

    const auto uploadHeapProp = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
    const auto uploadResourceDesc = CD3DX12_RESOURCE_DESC::Buffer( m_PageSize );
    ThrowIfFailed( d3d12Device->CreateCommittedResource(
        &uploadHeapProp, D3D12_HEAP_FLAG_NONE,
        &uploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS( &m_d3d12Resource ) ) );

    m_d3d12Resource->SetName( L"Upload Buffer (Page)" );

    m_GPUPtr = m_d3d12Resource->GetGPUVirtualAddress();
    m_d3d12Resource->Map( 0, nullptr, &m_CPUPtr );
}

UploadBuffer::Page::~Page()
{
    m_d3d12Resource->Unmap( 0, nullptr );
    m_CPUPtr = nullptr;
    m_GPUPtr = D3D12_GPU_VIRTUAL_ADDRESS( 0 );
}

bool UploadBuffer::Page::HasSpace( size_t sizeInBytes, size_t alignment ) const
{
    size_t alignedSize   = Math::AlignUp( sizeInBytes, alignment );
    size_t alignedOffset = Math::AlignUp( m_Offset, alignment );

    return alignedOffset + alignedSize <= m_PageSize;
}

UploadBuffer::Allocation UploadBuffer::Page::Allocate( size_t sizeInBytes, size_t alignment )
{
    if ( !HasSpace( sizeInBytes, alignment ) )
    {
        // Can't allocate space from page.
        throw std::bad_alloc();
    }

    size_t alignedSize = Math::AlignUp( sizeInBytes, alignment );
    m_Offset           = Math::AlignUp( m_Offset, alignment );

    Allocation allocation;
    allocation.CPU = static_cast<uint8_t*>( m_CPUPtr ) + m_Offset;
    allocation.GPU = m_GPUPtr + m_Offset;

    m_Offset += alignedSize;

    return allocation;
}

void UploadBuffer::Page::Reset()
{
    m_Offset = 0;
}