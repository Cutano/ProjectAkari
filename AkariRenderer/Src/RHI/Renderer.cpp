#include "pch.h"
#include "Renderer.h"

#include "Device.h"
#include "SwapChain.h"
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
}
