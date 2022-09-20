#include "pch.h"

#include "DynamicDescriptorHeap.h"

#include "CommandList.h"
#include "Device.h"
#include "RootSignature.h"

using namespace Akari;

DynamicDescriptorHeap::DynamicDescriptorHeap( Device& device, D3D12_DESCRIPTOR_HEAP_TYPE heapType,
                                              uint32_t numDescriptorsPerHeap )
: m_Device( device )
, m_DescriptorHeapType( heapType )
, m_NumDescriptorsPerHeap( numDescriptorsPerHeap )
, m_DescriptorTableBitMask( 0 )
, m_StaleDescriptorTableBitMask( 0 )
, m_StaleCBVBitMask( 0 )
, m_StaleSRVBitMask( 0 )
, m_StaleUAVBitMask( 0 )
, m_CurrentCPUDescriptorHandle( D3D12_DEFAULT )
, m_CurrentGPUDescriptorHandle( D3D12_DEFAULT )
, m_NumFreeHandles( 0 )
{
    m_DescriptorHandleIncrementSize = m_Device.GetDescriptorHandleIncrementSize( heapType );

    // Allocate space for staging CPU visible descriptors.
    m_DescriptorHandleCache = std::make_unique<D3D12_CPU_DESCRIPTOR_HANDLE[]>( m_NumDescriptorsPerHeap );
}

DynamicDescriptorHeap::~DynamicDescriptorHeap() {}

void DynamicDescriptorHeap::ParseRootSignature( const std::shared_ptr<RootSignature>& rootSignature )
{
    assert( rootSignature );

    // If the root signature changes, all descriptors must be (re)bound to the
    // command list.
    m_StaleDescriptorTableBitMask = 0;

    const auto& rootSignatureDesc = rootSignature->GetRootSignatureDesc();

    // Get a bit mask that represents the root parameter indices that match the
    // descriptor heap type for this dynamic descriptor heap.
    m_DescriptorTableBitMask        = rootSignature->GetDescriptorTableBitMask( m_DescriptorHeapType );
    uint32_t descriptorTableBitMask = m_DescriptorTableBitMask;

    uint32_t currentOffset = 0;
    DWORD    rootIndex;
    while ( _BitScanForward( &rootIndex, descriptorTableBitMask ) && rootIndex < rootSignatureDesc.NumParameters )
    {
        uint32_t numDescriptors = rootSignature->GetNumDescriptors( rootIndex );

        DescriptorTableCache& descriptorTableCache = m_DescriptorTableCache[rootIndex];
        descriptorTableCache.NumDescriptors        = numDescriptors;
        descriptorTableCache.BaseDescriptor        = m_DescriptorHandleCache.get() + currentOffset;

        currentOffset += numDescriptors;

        // Flip the descriptor table bit so it's not scanned again for the current index.
        descriptorTableBitMask ^= ( 1 << rootIndex );
    }

    // Make sure the maximum number of descriptors per descriptor heap has not been exceeded.
    assert(
        currentOffset <= m_NumDescriptorsPerHeap &&
        "The root signature requires more than the maximum number of descriptors per descriptor heap. Consider increasing the maximum number of descriptors per descriptor heap." );
}

void DynamicDescriptorHeap::StageDescriptors( uint32_t rootParameterIndex, uint32_t offset, uint32_t numDescriptors,
                                              const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptor )
{
    // Cannot stage more than the maximum number of descriptors per heap.
    // Cannot stage more than MaxDescriptorTables root parameters.
    if ( numDescriptors > m_NumDescriptorsPerHeap || rootParameterIndex >= MaxDescriptorTables )
    {
        throw std::bad_alloc();
    }

    DescriptorTableCache& descriptorTableCache = m_DescriptorTableCache[rootParameterIndex];

    // Check that the number of descriptors to copy does not exceed the number
    // of descriptors expected in the descriptor table.
    if ( ( offset + numDescriptors ) > descriptorTableCache.NumDescriptors )
    {
        throw std::length_error( "Number of descriptors exceeds the number of descriptors in the descriptor table." );
    }

    D3D12_CPU_DESCRIPTOR_HANDLE* dstDescriptor = ( descriptorTableCache.BaseDescriptor + offset );
    for ( uint32_t i = 0; i < numDescriptors; ++i )
    { dstDescriptor[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE( srcDescriptor, i, m_DescriptorHandleIncrementSize ); }

    // Set the root parameter index bit to make sure the descriptor table
    // at that index is bound to the command list.
    m_StaleDescriptorTableBitMask |= ( 1 << rootParameterIndex );
}

void DynamicDescriptorHeap::StageInlineCBV( uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation )
{
    assert( rootParameterIndex < MaxDescriptorTables );

    m_InlineCBV[rootParameterIndex] = bufferLocation;
    m_StaleCBVBitMask |= ( 1 << rootParameterIndex );
}

void DynamicDescriptorHeap::StageInlineSRV( uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation )
{
    assert( rootParameterIndex < MaxDescriptorTables );

    m_InlineSRV[rootParameterIndex] = bufferLocation;
    m_StaleSRVBitMask |= ( 1 << rootParameterIndex );
}

void DynamicDescriptorHeap::StageInlineUAV( uint32_t rootParamterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation )
{
    assert( rootParamterIndex < MaxDescriptorTables );

    m_InlineUAV[rootParamterIndex] = bufferLocation;
    m_StaleUAVBitMask |= ( 1 << rootParamterIndex );
}

uint32_t DynamicDescriptorHeap::ComputeStaleDescriptorCount() const
{
    uint32_t numStaleDescriptors = 0;
    DWORD    i;
    DWORD    staleDescriptorsBitMask = m_StaleDescriptorTableBitMask;

    while ( _BitScanForward( &i, staleDescriptorsBitMask ) )
    {
        numStaleDescriptors += m_DescriptorTableCache[i].NumDescriptors;
        staleDescriptorsBitMask ^= ( 1 << i );
    }

    return numStaleDescriptors;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DynamicDescriptorHeap::RequestDescriptorHeap()
{
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    if ( !m_AvailableDescriptorHeaps.empty() )
    {
        descriptorHeap = m_AvailableDescriptorHeaps.front();
        m_AvailableDescriptorHeaps.pop();
    }
    else
    {
        descriptorHeap = CreateDescriptorHeap();
        m_DescriptorHeapPool.push( descriptorHeap );
    }

    return descriptorHeap;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DynamicDescriptorHeap::CreateDescriptorHeap()
{
    auto d3d12Device = m_Device.GetD3D12Device();

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
    descriptorHeapDesc.Type                       = m_DescriptorHeapType;
    descriptorHeapDesc.NumDescriptors             = m_NumDescriptorsPerHeap;
    descriptorHeapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    ThrowIfFailed( d3d12Device->CreateDescriptorHeap( &descriptorHeapDesc, IID_PPV_ARGS( &descriptorHeap ) ) );

    return descriptorHeap;
}

void DynamicDescriptorHeap::CommitDescriptorTables(
    CommandList&                                                                         commandList,
    std::function<void( ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE )> setFunc )
{
    // Compute the number of descriptors that need to be copied
    uint32_t numDescriptorsToCommit = ComputeStaleDescriptorCount();

    if ( numDescriptorsToCommit > 0 )
    {
        auto d3d12Device              = m_Device.GetD3D12Device();
        auto d3d12GraphicsCommandList = commandList.GetD3D12CommandList().Get();
        assert( d3d12GraphicsCommandList != nullptr );

        if ( !m_CurrentDescriptorHeap || m_NumFreeHandles < numDescriptorsToCommit )
        {
            m_CurrentDescriptorHeap      = RequestDescriptorHeap();
            m_CurrentCPUDescriptorHandle = m_CurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            m_CurrentGPUDescriptorHandle = m_CurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
            m_NumFreeHandles             = m_NumDescriptorsPerHeap;

            commandList.SetDescriptorHeap( m_DescriptorHeapType, m_CurrentDescriptorHeap.Get() );

            // When updating the descriptor heap on the command list, all descriptor
            // tables must be (re)recopied to the new descriptor heap (not just
            // the stale descriptor tables).
            m_StaleDescriptorTableBitMask = m_DescriptorTableBitMask;
        }

        DWORD rootIndex;
        // Scan from LSB to MSB for a bit set in staleDescriptorsBitMask
        while ( _BitScanForward( &rootIndex, m_StaleDescriptorTableBitMask ) )
        {
            UINT                         numSrcDescriptors     = m_DescriptorTableCache[rootIndex].NumDescriptors;
            D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorHandles = m_DescriptorTableCache[rootIndex].BaseDescriptor;

            D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRangeStarts[] = { m_CurrentCPUDescriptorHandle };
            UINT                        pDestDescriptorRangeSizes[]  = { numSrcDescriptors };

            // Copy the staged CPU visible descriptors to the GPU visible descriptor heap.
            d3d12Device->CopyDescriptors( 1, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes, numSrcDescriptors,
                                          pSrcDescriptorHandles, nullptr, m_DescriptorHeapType );

            // Set the descriptors on the command list using the passed-in setter function.
            setFunc( d3d12GraphicsCommandList, rootIndex, m_CurrentGPUDescriptorHandle );

            // Offset current CPU and GPU descriptor handles.
            m_CurrentCPUDescriptorHandle.Offset( numSrcDescriptors, m_DescriptorHandleIncrementSize );
            m_CurrentGPUDescriptorHandle.Offset( numSrcDescriptors, m_DescriptorHandleIncrementSize );
            m_NumFreeHandles -= numSrcDescriptors;

            // Flip the stale bit so the descriptor table is not recopied again unless it is updated with a new
            // descriptor.
            m_StaleDescriptorTableBitMask ^= ( 1 << rootIndex );
        }
    }
}

void DynamicDescriptorHeap::CommitInlineDescriptors(
    CommandList& commandList, const D3D12_GPU_VIRTUAL_ADDRESS* bufferLocations, uint32_t& bitMask,
    std::function<void( ID3D12GraphicsCommandList*, UINT, D3D12_GPU_VIRTUAL_ADDRESS )> setFunc )
{
    if ( bitMask != 0 )
    {
        auto  d3d12GraphicsCommandList = commandList.GetD3D12CommandList().Get();
        DWORD rootIndex;
        while ( _BitScanForward( &rootIndex, bitMask ) )
        {
            setFunc( d3d12GraphicsCommandList, rootIndex, bufferLocations[rootIndex] );

            // Flip the stale bit so the descriptor is not recopied again unless it is updated with a new descriptor.
            bitMask ^= ( 1 << rootIndex );
        }
    }
}

void DynamicDescriptorHeap::CommitStagedDescriptorsForDraw( CommandList& commandList )
{
    CommitDescriptorTables( commandList, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable );
    CommitInlineDescriptors( commandList, m_InlineCBV, m_StaleCBVBitMask,
                             &ID3D12GraphicsCommandList::SetGraphicsRootConstantBufferView );
    CommitInlineDescriptors( commandList, m_InlineSRV, m_StaleSRVBitMask,
                             &ID3D12GraphicsCommandList::SetGraphicsRootShaderResourceView );
    CommitInlineDescriptors( commandList, m_InlineUAV, m_StaleUAVBitMask,
                             &ID3D12GraphicsCommandList::SetGraphicsRootUnorderedAccessView );
}

void DynamicDescriptorHeap::CommitStagedDescriptorsForDispatch( CommandList& commandList )
{
    CommitDescriptorTables( commandList, &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable );
    CommitInlineDescriptors( commandList, m_InlineCBV, m_StaleCBVBitMask,
                             &ID3D12GraphicsCommandList::SetComputeRootConstantBufferView );
    CommitInlineDescriptors( commandList, m_InlineSRV, m_StaleSRVBitMask,
                             &ID3D12GraphicsCommandList::SetComputeRootShaderResourceView );
    CommitInlineDescriptors( commandList, m_InlineUAV, m_StaleUAVBitMask,
                             &ID3D12GraphicsCommandList::SetComputeRootUnorderedAccessView );
}

D3D12_GPU_DESCRIPTOR_HANDLE DynamicDescriptorHeap::CopyDescriptor( CommandList&                commandList,
                                                                   D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor )
{
    if ( !m_CurrentDescriptorHeap || m_NumFreeHandles < 1 )
    {
        m_CurrentDescriptorHeap      = RequestDescriptorHeap();
        m_CurrentCPUDescriptorHandle = m_CurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        m_CurrentGPUDescriptorHandle = m_CurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
        m_NumFreeHandles             = m_NumDescriptorsPerHeap;

        commandList.SetDescriptorHeap( m_DescriptorHeapType, m_CurrentDescriptorHeap.Get() );

        // When updating the descriptor heap on the command list, all descriptor
        // tables must be (re)recopied to the new descriptor heap (not just
        // the stale descriptor tables).
        m_StaleDescriptorTableBitMask = m_DescriptorTableBitMask;
    }

    auto d3d12Device = m_Device.GetD3D12Device();

    D3D12_GPU_DESCRIPTOR_HANDLE hGPU = m_CurrentGPUDescriptorHandle;
    d3d12Device->CopyDescriptorsSimple( 1, m_CurrentCPUDescriptorHandle, cpuDescriptor, m_DescriptorHeapType );

    m_CurrentCPUDescriptorHandle.Offset( 1, m_DescriptorHandleIncrementSize );
    m_CurrentGPUDescriptorHandle.Offset( 1, m_DescriptorHandleIncrementSize );
    m_NumFreeHandles -= 1;

    return hGPU;
}

void DynamicDescriptorHeap::Reset()
{
    m_AvailableDescriptorHeaps = m_DescriptorHeapPool;
    m_CurrentDescriptorHeap.Reset();
    m_CurrentCPUDescriptorHandle  = CD3DX12_CPU_DESCRIPTOR_HANDLE( D3D12_DEFAULT );
    m_CurrentGPUDescriptorHandle  = CD3DX12_GPU_DESCRIPTOR_HANDLE( D3D12_DEFAULT );
    m_NumFreeHandles              = 0;
    m_DescriptorTableBitMask      = 0;
    m_StaleDescriptorTableBitMask = 0;
    m_StaleCBVBitMask             = 0;
    m_StaleSRVBitMask             = 0;
    m_StaleUAVBitMask             = 0;

    // Reset the descriptor cache
    for ( int i = 0; i < MaxDescriptorTables; ++i )
    {
        m_DescriptorTableCache[i].Reset();
        m_InlineCBV[i] = 0ull;
        m_InlineSRV[i] = 0ull;
        m_InlineUAV[i] = 0ull;
    }
}
