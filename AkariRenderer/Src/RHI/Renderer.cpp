#include "pch.h"
#include "Renderer.h"

#include "CommandQueue.h"
#include "Device.h"
#include "SwapChain.h"
#include "CommandList.h"
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
    }

    void Renderer::ShutDown()
    {
        m_SwapChain.reset();
        m_Device.reset();

        atexit(&Device::ReportLiveObjects);
    }

    void Renderer::LoadScene(std::wstring path)
    {
        auto& commandQueue = m_Device->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_COPY );
        auto  commandList  = commandQueue.GetCommandList();

        auto scene = commandList->LoadSceneFromFile(path);
    }

    void Renderer::OnUpdate(DeltaTime dt)
    {
        
    }

    void Renderer::OnResize() const
    {
        const auto window = &Application::Get().GetWindow();
        const auto width = window->GetWidth();
        const auto height = window->GetHeight();

        // Flush any pending commands before resizing resources.
        m_Device->Flush();
        
        m_SwapChain->Resize(width, height);
        
        spdlog::info("Window resized to {0}, {1}.", width, height);
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

    void Renderer::ExecuteCommandList(std::shared_ptr<CommandList> commandList) const
    {
        m_Device->GetCommandQueue(commandList->GetCommandListType()).ExecuteCommandList(commandList);
    }
}
