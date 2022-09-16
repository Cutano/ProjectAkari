#include "pch.h"
#include "Renderer.h"

#include "Application/Application.h"
#include "Window/WindowsWindow.h"

using namespace Microsoft::WRL;

extern "C" {
__declspec(dllexport) extern constexpr UINT D3D12SDKVersion = 606;
}

extern "C" {
__declspec(dllexport) extern const char8_t* D3D12SDKPath = u8".\\D3D12\\";
}

// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, *ppAdapter will be set to nullptr.
_Use_decl_annotations_ void GetHardwareAdapter(IDXGIFactory7* pFactory, IDXGIAdapter1** ppAdapter)
{
    *ppAdapter = nullptr;

    ComPtr<IDXGIAdapter1> adapter;
    ComPtr<IDXGIFactory7> factory7;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory7))))
    {
        for (
            UINT adapterIndex = 0;
            SUCCEEDED(factory7->EnumAdapterByGpuPreference(
                adapterIndex,
                DXGI_GPU_PREFERENCE_UNSPECIFIED,
                IID_PPV_ARGS(&adapter)));
            ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device9), nullptr)))
            {
                break;
            }
        }
    }

    if (adapter.Get() == nullptr)
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device9), nullptr)))
            {
                break;
            }
        }
    }

    *ppAdapter = adapter.Detach();
}

namespace Akari
{
    void Renderer::Init()
    {
        DWORD dxgiFactoryFlags = 0;
#if defined(_DEBUG)
        // Enable the D3D12 debug layer.
        {
            spdlog::info("D3D12 SDK version: {0}", D3D12SDKVersion);
            ComPtr<ID3D12Debug6> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
                spdlog::info("D3D12 debug validation layer enabled.");

                debugController->SetEnableGPUBasedValidation(true);
                spdlog::info("GPU based validation enabled, could slow down performance.");
            }
            else
            {
                spdlog::warn("Unable to enable D3D12 debug validation layer!");
            }

            ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
            {
                dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

                DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
                {
                    // IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides.
                    80,
                };
                DXGI_INFO_QUEUE_FILTER filter = {};
                filter.DenyList.NumIDs = _countof(hide);
                filter.DenyList.pIDList = hide;
                dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
            }
        }
#endif

        ComPtr<IDXGIFactory7> factory;
        ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);

        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_Device)
        ));

#if defined(_DEBUG)
        {
            ComPtr<ID3D12InfoQueue1> d3d12InfoQueue;
            if (SUCCEEDED(m_Device->QueryInterface(IID_PPV_ARGS(&d3d12InfoQueue))))
            {
                // Suppress messages based on their severity level
                D3D12_MESSAGE_SEVERITY Severities[] =
                {
                    D3D12_MESSAGE_SEVERITY_INFO
                };

                // Suppress individual messages by their ID
                // D3D12_MESSAGE_ID DenyIds[] =
                // {
                //     // This occurs when there are uninitialized descriptors in a descriptor table, even when a
                //     // shader does not access the missing descriptors.  I find this is common when switching
                //     // shader permutations and not wanting to change much code to reorder resources.
                //     D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,
                //
                //     // Triggered when a shader does not export all color components of a render target, such as
                //     // when only writing RGB to an R10G10B10A2 buffer, ignoring alpha.
                //     D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_PS_OUTPUT_RT_OUTPUT_MISMATCH,
                //
                //     // This occurs when a descriptor table is unbound even when a shader does not access the missing
                //     // descriptors.  This is common with a root signature shared between disparate shaders that
                //     // don't all need the same types of resources.
                //     D3D12_MESSAGE_ID_COMMAND_LIST_DESCRIPTOR_TABLE_NOT_SET,
                //
                //     D3D12_MESSAGE_ID_RESOURCE_BARRIER_DUPLICATE_SUBRESOURCE_TRANSITIONS,
                // };

                D3D12_INFO_QUEUE_FILTER NewFilter = {};
                //NewFilter.DenyList.NumCategories = _countof(Categories);
                //NewFilter.DenyList.pCategoryList = Categories;
                NewFilter.DenyList.NumSeverities = _countof(Severities);
                NewFilter.DenyList.pSeverityList = Severities;
                // NewFilter.DenyList.NumIDs = _countof(DenyIds);
                // NewFilter.DenyList.pIDList = DenyIds;

                d3d12InfoQueue->PushStorageFilter(&NewFilter);
            }
        }
#endif

        // Describe and create the command queue.
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueueDirect)));

        const auto* window = dynamic_cast<WindowsWindow*>(&Application::Get().GetWindow());

        // Describe and create the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = FrameCount;
        swapChainDesc.Width = window->GetWidth();
        swapChainDesc.Height = window->GetHeight();
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;
        // swapChainDesc.Windowed = TRUE; // TODO: Use window props instead.

        ComPtr<IDXGISwapChain1> swapChain;
        ThrowIfFailed(factory->CreateSwapChainForHwnd(
            m_CommandQueueDirect.Get(), // Swap chain needs the queue so that it can force a flush on it.
            window->GetHandle(),
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain
        ));

        ThrowIfFailed(swapChain.As(&m_SwapChain));

        // TODO: Support fullscreen transitions.
        ThrowIfFailed(factory->MakeWindowAssociation(window->GetHandle(), DXGI_MWA_NO_ALT_ENTER));

        m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

        // Create descriptor heaps.
        {
            // Describe and create a render target view (RTV) descriptor heap.
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = FrameCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            ThrowIfFailed(m_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvHeap)));

            m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }

        // Create frame resources.
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());

            // Create a RTV for each frame.
            for (UINT n = 0; n < FrameCount; n++)
            {
                ThrowIfFailed(m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTargets[n])));
                m_Device->CreateRenderTargetView(m_RenderTargets[n].Get(), nullptr, rtvHandle);
                rtvHandle.Offset(1, m_RtvDescriptorSize);
            }
        }
    }

    void Renderer::ShutDown()
    {
// #if defined(_DEBUG)
//         IDXGIDebug1* dxgiDebug;
//         ThrowIfFailed(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)));
//
//         dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
// #endif
    }
}
