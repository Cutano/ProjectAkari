#pragma once

#include "Window.h"

#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

namespace Akari {

    class WindowsWindow : public Window
    {
    public:
        WindowsWindow(const WindowSpecification& specification);
        ~WindowsWindow() override;

        void Init() override;
        void ProcessEvents() override;
        void SwapBuffers() override;

        uint32_t GetWidth() const override { return m_Data.Width; }
        uint32_t GetHeight() const override { return m_Data.Height; }

        std::pair<uint32_t, uint32_t> GetSize() const override { return { m_Data.Width, m_Data.Height }; }
        std::pair<float, float> GetWindowPos() const override;

        // Window attributes
        void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
        void SetVSync(bool enabled) override;
        bool IsVSync() const override;
        void SetResizable(bool resizable) const override;

        void Maximize() override;
        void CenterWindow() override;

        const std::string& GetTitle() const override { return m_Data.Title; }
        void SetTitle(const std::string& title) override;

        [[nodiscard]] void* GetNativeWindow() const override { return m_Window; }
        [[nodiscard]] HWND GetHandle() const { return m_MainWnd; }

        // virtual Ref<RendererContext> GetRenderContext() override { return m_RendererContext; }
        // virtual VulkanSwapChain& GetSwapChain() override;
    private:
        virtual void Shutdown();
        
    private:
        HWND m_MainWnd;
        
        GLFWwindow* m_Window;
        GLFWcursor* m_ImGuiMouseCursors[9] = { 0 };
        WindowSpecification m_Specification;
        struct WindowData
        {
            std::string Title;
            uint32_t Width, Height;

            EventCallbackFn EventCallback;
        };

        WindowData m_Data;
        float m_LastFrameTime = 0.0f;

        // Ref<RendererContext> m_RendererContext;
        // VulkanSwapChain m_SwapChain;
    };

}