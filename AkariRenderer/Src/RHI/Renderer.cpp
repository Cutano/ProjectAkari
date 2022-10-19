#include "pch.h"
#include "Renderer.h"

#include <future>

#include "CommandQueue.h"
#include "Device.h"
#include "SwapChain.h"
#include "CommandList.h"
#include "Texture.h"
#include "Application/Application.h"
#include "Layers/ImGuiLayer.h"
#include "Window/WindowsWindow.h"
#include "RPI/RenderPipeline.h"
#include "RPI/RenderContext.h"

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

        // Check the best multisample quality level that can be used for the given back buffer format.
        DXGI_SAMPLE_DESC sampleDesc = m_Device->GetMultisampleQualityLevels(m_SwapChain->GetRenderTargetFormat());
        auto colorDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            m_SwapChain->GetRenderTargetFormat(),
            m_SwapChain->GetRenderTarget().GetWidth(),
            m_SwapChain->GetRenderTarget().GetHeight(),
            1, 1, sampleDesc.Count,
            sampleDesc.Quality,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET );
        D3D12_CLEAR_VALUE colorClearValue {colorDesc.Format, {0, 0, 0, 1.0f}};
        auto msaaTexture = m_Device->CreateTexture( colorDesc, &colorClearValue );
        msaaTexture->SetName( L"Main MSAA Render Target" );

        // Attach the texture to the render target.
        m_MsaaRenderTarget = std::make_shared<RenderTarget>();
        m_MsaaRenderTarget->AttachTexture(Color0, msaaTexture);

        m_ImGuiLayer = std::make_shared<ImGuiLayer>();
        m_ImGuiLayer->OnAttach();
    }

    std::future<void> Renderer::PrepareFirstFrameAsync() const
    {
        return std::async(std::launch::async, [this]
        {
            spdlog::info("Initializing render pipeline...");
            CoInitialize(nullptr);
            m_RenderPipeline->Prepare();
        });
    }

    void Renderer::ShutDown()
    {
        m_ImGuiLayer->OnDetach();
        m_MsaaRenderTarget.reset();
        m_SwapChain.reset();
        m_Device.reset();

        atexit(&Device::ReportLiveObjects);
    }

    std::shared_ptr<Model> Renderer::LoadModel(std::wstring path)
    {
        auto& commandQueue = m_Device->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_COPY );
        auto  commandList  = commandQueue.GetCommandList();

        return commandList->LoadModelFromFile(path);
    }

    void Renderer::OnUpdate(RenderContext& context)
    {
        {
            const auto cmd = GetCommandListDirect();
            constexpr FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
            cmd->ClearTexture(m_SwapChain->GetRenderTarget().GetTexture(Color0), clearColor);
            cmd->ClearTexture(m_MsaaRenderTarget->GetTexture(Color0), clearColor);
            cmd->SetRenderTarget(*m_MsaaRenderTarget);
            ExecuteCommandList(cmd);
        }
        
        m_RenderPipeline->Render(context);

        if (m_ImGuiLayer != nullptr)
        {
            m_ImGuiLayer->OnUpdate(context);
        }
        else
        {
            spdlog::warn("No GUI Layer found, rendering without GUI!");
        }

        // Resolve the MSAA render target to the swapchain's backbuffer.
        {
            const auto cmd = GetCommandListDirect();
            auto swapChainBackBuffer = m_SwapChain->GetRenderTarget().GetTexture(Color0);
            auto msaaRenderTarget = m_MsaaRenderTarget->GetTexture(Color0);
            cmd->ResolveSubresource(swapChainBackBuffer, msaaRenderTarget);
            ExecuteCommandList(cmd);
        }
        m_SwapChain->Present();
    }

    void Renderer::OnResize(uint32_t width, uint32_t height) const
    {
        // Flush any pending commands before resizing resources.
        m_Device->Flush();
        
        m_SwapChain->Resize(width, height);
        m_MsaaRenderTarget->Resize(width, height);
        
        spdlog::trace("Window resized to {0}, {1}.", width, height);
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

    std::shared_ptr<RenderTarget> Renderer::GetMsaaRenderTarget() const
    {
        return m_MsaaRenderTarget;
    }

    std::shared_ptr<ImGuiLayer> Renderer::GetImGuiLayer() const
    {
        return m_ImGuiLayer;
    }

    std::shared_ptr<RenderPipeline> Renderer::GetRenderPipeline() const
    {
        return m_RenderPipeline;
    }

    void Renderer::SetRenderPipeline(const std::shared_ptr<RenderPipeline>& rp)
    {
        m_RenderPipeline = rp;
    }

    uint64_t Renderer::ExecuteCommandList(std::shared_ptr<CommandList> commandList) const
    {
        return m_Device->GetCommandQueue(commandList->GetCommandListType()).ExecuteCommandList(commandList);
    }

    void Renderer::ExecuteAndFlushCommandList(std::shared_ptr<CommandList> commandList) const
    {
        auto& queue = m_Device->GetCommandQueue(commandList->GetCommandListType());
        queue.ExecuteCommandList(commandList);
        queue.Flush();
    }
}
