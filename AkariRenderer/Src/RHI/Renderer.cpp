#include "pch.h"
#include "Renderer.h"

#include "CommandQueue.h"
#include "Device.h"
#include "SwapChain.h"
#include "CommandList.h"
#include "Texture.h"
#include "Application/Application.h"
#include "Window/WindowsWindow.h"

using namespace Microsoft::WRL;

extern "C" {
__declspec(dllexport) extern constexpr UINT D3D12SDKVersion = 606;
}

extern "C" {
__declspec(dllexport) extern const char8_t* D3D12SDKPath = u8".\\D3D12\\";
}

namespace Akari
{
    void Renderer::Init()
    {
        if (m_Device != nullptr)
        {
            spdlog::error("Device has already been created! Please do not call Renderer::Init() twice.");
            throw std::runtime_error("Device has already been created!");
        }

        Device::EnableDebugLayer();
        m_Device = Device::Create();

        const auto description = m_Device->GetDescription();
        spdlog::info(ConvertString(L"Device Created: " + description));

        const auto handle = dynamic_cast<WindowsWindow*>(&Application::Get().GetWindow())->GetHandle();
        m_SwapChain = m_Device->CreateSwapChain(handle);
        m_SwapChain->SetVSync(Application::Get().GetWindow().IsVSync());
        // TODO: Set isFullScreen as well.

        const auto sceneFrameBufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R11G11B10_FLOAT, m_SwapChain->GetRenderTarget().GetWidth(), m_SwapChain->GetRenderTarget().GetHeight());
        const auto sceneDepthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_SwapChain->GetRenderTarget().GetWidth(), m_SwapChain->GetRenderTarget().GetHeight());
        m_SceneFrameBuffer = m_Device->CreateTexture(sceneFrameBufferDesc);
        m_SceneDepth = m_Device->CreateTexture(sceneDepthStencilDesc);

        m_SceneRenderTarget = std::make_shared<RenderTarget>();
        m_SceneRenderTarget->AttachTexture(Color0, m_SceneFrameBuffer);
        m_SceneRenderTarget->AttachTexture(DepthStencil, m_SceneDepth);
    }

    void Renderer::ShutDown()
    {
        m_SceneDepth.reset();
        m_SceneFrameBuffer.reset();
        m_SceneRenderTarget.reset();
        m_SwapChain.reset();
        m_Device.reset();

        atexit(&Device::ReportLiveObjects);
    }

    void Renderer::LoadModel(std::wstring path)
    {
        auto& commandQueue = m_Device->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_COPY );
        auto  commandList  = commandQueue.GetCommandList();

        auto model = commandList->LoadModelFromFile(path);
    }

    void Renderer::OnUpdate(DeltaTime dt)
    {
        
    }

    void Renderer::OnResize(uint32_t width, uint32_t height) const
    {
        // Flush any pending commands before resizing resources.
        m_Device->Flush();
        
        m_SwapChain->Resize(width, height);
        
        spdlog::info("Window resized to {0}, {1}.", width, height);
    }

    void Renderer::OnSceneResize(float width, float height) const
    {
        m_SceneRenderTarget->Resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

        spdlog::info("Scene resized to {0}, {1}.", width, height);
    }

    std::shared_ptr<CommandList> Renderer::GetCommandListDirect() const
    {
        return m_Device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT).GetCommandList();
    }

    std::shared_ptr<CommandList> Renderer::GetCommandListCopy() const
    {
        return m_Device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY).GetCommandList();
    }

    std::shared_ptr<CommandList> Renderer::GetCommandListCompute() const
    {
        return m_Device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE).GetCommandList();
    }

    std::shared_ptr<Device> Renderer::GetDevice() const
    {
        return m_Device;
    }

    std::shared_ptr<SwapChain> Renderer::GetSwapChain() const
    {
        return m_SwapChain;
    }

    uint64_t Renderer::ExecuteCommandList(std::shared_ptr<CommandList> commandList) const
    {
        return m_Device->GetCommandQueue(commandList->GetCommandListType()).ExecuteCommandList(commandList);
    }
}
