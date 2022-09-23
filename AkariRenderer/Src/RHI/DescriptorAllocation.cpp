#include "pch.h"

#include "DescriptorAllocation.h"

#include "DescriptorAllocatorPage.h"

using namespace Akari;

DescriptorAllocation::DescriptorAllocation()
    : m_CPUDescriptor{ 0 }
    , m_NumHandles( 0 )
    , m_DescriptorSize( 0 )
    , m_Page( nullptr )
{}

DescriptorAllocation::DescriptorAllocation( D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor, D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptor, uint32_t numHandles, uint32_t descriptorSize, std::shared_ptr<DescriptorAllocatorPage> page )
    : m_CPUDescriptor( cpuDescriptor )
    , m_GPUDescriptor( gpuDescriptor )
    , m_NumHandles( numHandles )
    , m_DescriptorSize( descriptorSize )
    , m_Page( page )
{}


DescriptorAllocation::~DescriptorAllocation()
{
    Free();
}

DescriptorAllocation::DescriptorAllocation( DescriptorAllocation&& allocation ) noexcept
    : m_CPUDescriptor(allocation.m_CPUDescriptor)
    , m_NumHandles(allocation.m_NumHandles)
    , m_DescriptorSize(allocation.m_DescriptorSize)
    , m_Page(std::move(allocation.m_Page))
{
    allocation.m_CPUDescriptor.ptr = 0;
    allocation.m_NumHandles = 0;
    allocation.m_DescriptorSize = 0;
}

DescriptorAllocation& DescriptorAllocation::operator=( DescriptorAllocation&& other ) noexcept
{
    // Free this descriptor if it points to anything.
    Free();

    m_CPUDescriptor = other.m_CPUDescriptor;
    m_NumHandles = other.m_NumHandles;
    m_DescriptorSize = other.m_DescriptorSize;
    m_Page = std::move( other.m_Page );

    other.m_CPUDescriptor.ptr = 0;
    other.m_NumHandles = 0;
    other.m_DescriptorSize = 0;

    return *this;
}

void DescriptorAllocation::Free()
{
    if ( !IsNull() && m_Page )
    {
        m_Page->Free( std::move( *this ) );
        
        m_CPUDescriptor.ptr = 0;
        m_NumHandles = 0;
        m_DescriptorSize = 0;
        m_Page.reset();
    }
}

// Check if this a valid descriptor.
bool DescriptorAllocation::IsNull() const
{
    return m_CPUDescriptor.ptr == 0;
}

// Get a descriptor at a particular offset in the allocation.
D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocation::GetDescriptorHandle( uint32_t offset ) const
{
    assert( offset < m_NumHandles );
    return { m_CPUDescriptor.ptr + ( m_DescriptorSize * offset ) };
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorAllocation::GetGPUDescriptorHandle(uint32_t offset) const
{
    assert( offset < m_NumHandles );
    return { m_GPUDescriptor.ptr + ( m_DescriptorSize * offset ) };
}

uint32_t DescriptorAllocation::GetNumHandles() const
{
    return m_NumHandles;
}

std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocation::GetDescriptorAllocatorPage() const
{
    return m_Page;
}
