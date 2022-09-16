#include "pch.h"
#include "WindowsWindow.h"

#include "Application/Application.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Input/Input.h"

#include <imgui.h>
// #include "stb_image.h"

namespace Akari {

	static void GLFWErrorCallback(int error, const char* description)
	{
		spdlog::error("GLFW Error ({0}): {1}", error, description);
	}

	static bool s_GLFWInitialized = false;

	Window* Window::Create(const WindowSpecification& specification)
	{
		return new WindowsWindow(specification);
	}

	WindowsWindow::WindowsWindow(const WindowSpecification& props)
		: m_Specification(props)
	{
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init()
	{
		m_Data.Title = m_Specification.Title;
		m_Data.Width = m_Specification.Width;
		m_Data.Height = m_Specification.Height;
		
		spdlog::info("Creating window {0} ({1}, {2})", m_Specification.Title, m_Specification.Width, m_Specification.Height);

		if (!s_GLFWInitialized)
		{
			// TODO: glfwTerminate on system shutdown
			int success = glfwInit();
			assert(success);
			glfwSetErrorCallback(GLFWErrorCallback);

			s_GLFWInitialized = true;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		if (!m_Specification.Decorated)
		{
			// This removes titlebar on all platforms
			// and all of the native window effects on non-Windows platforms
			glfwWindowHint(GLFW_DECORATED, false);
		}

		if (m_Specification.Fullscreen)
		{
			GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

			glfwWindowHint(GLFW_DECORATED, false);
			glfwWindowHint(GLFW_RED_BITS, mode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

			m_Window = glfwCreateWindow(mode->width, mode->height, m_Data.Title.c_str(), primaryMonitor, nullptr);
		}
		else
		{
			m_Window = glfwCreateWindow(static_cast<int>(m_Specification.Width), static_cast<int>(m_Specification.Height), m_Data.Title.c_str(), nullptr, nullptr);
		}

		m_MainWnd = glfwGetWin32Window(m_Window);

		// Create Renderer Context
		// m_RendererContext = RendererContext::Create();
		// m_RendererContext->Init();
		//
		// Ref<VulkanContext> context = m_RendererContext.As<VulkanContext>();
		//
		// m_SwapChain.Init(VulkanContext::GetInstance(), context->GetDevice());
		// m_SwapChain.InitSurface(m_Window);
		//
		// m_SwapChain.Create(&m_Data.Width, &m_Data.Height, m_Specification.VSync);
		//glfwMaximizeWindow(m_Window);
		glfwSetWindowUserPointer(m_Window, &m_Data);

		bool isRawMouseMotionSupported = glfwRawMouseMotionSupported();
		if (isRawMouseMotionSupported)
			glfwSetInputMode(m_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		else
			spdlog::warn("Platform", "Raw mouse motion not supported.");

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
			{
				auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

				WindowResizeEvent event((width), (height));
				data.EventCallback(event);
				data.Width = width;
				data.Height = height;
			});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

				WindowCloseEvent event;
				data.EventCallback(event);
			});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

				switch (action)
				{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(static_cast<KeyCode>(key), 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(static_cast<KeyCode>(key));
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(static_cast<KeyCode>(key), 1);
					data.EventCallback(event);
					break;
				}
				}
			});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, uint32_t codepoint)
			{
				auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

				KeyTypedEvent event(static_cast<KeyCode>(codepoint));
				data.EventCallback(event);
			});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
				auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

				switch (action)
				{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(static_cast<MouseButton>(button));
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(static_cast<MouseButton>(button));
					data.EventCallback(event);
					break;
				}
				}
			});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

				MouseScrolledEvent event((xOffset), (yOffset));
				data.EventCallback(event);
			});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double x, double y)
			{
				auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
				MouseMovedEvent event((x), (y));
				data.EventCallback(event);
			});

		// glfwSetTitlebarHitTestCallback(m_Window, [](GLFWwindow* window, int x, int y, int* hit)
		// 	{
		// 		auto& data = *((WindowData*)glfwGetWindowUserPointer(window));
		// 		WindowTitleBarHitTestEvent event(x, y, *hit);
		// 		data.EventCallback(event);
		// 	});

		glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow* window, int iconified)
		{
			auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
			WindowMinimizeEvent event(static_cast<bool>(iconified));
			data.EventCallback(event);
		});

		m_ImGuiMouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		m_ImGuiMouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
		m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);   // FIXME: GLFW doesn't have this.
		m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
		m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
		m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
		m_ImGuiMouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

		// Update window size to actual size
		{
			int width, height;
			glfwGetWindowSize(m_Window, &width, &height);
			m_Data.Width = width;
			m_Data.Height = height;
		}

		// Set icon
		// {
		// 	GLFWimage icon;
		// 	int channels;
		// 	icon.pixels = stbi_load("Resources/Editor/H_logo_square.png", &icon.width, &icon.height, &channels, 4);
		// 	glfwSetWindowIcon(m_Window, 1, &icon);
		// 	stbi_image_free(icon.pixels);
		// }
	}

	void WindowsWindow::Shutdown()
	{
		// m_SwapChain.Destroy();

		glfwTerminate();
		s_GLFWInitialized = false;
	}

	inline std::pair<float, float> WindowsWindow::GetWindowPos() const
	{
		int x, y;
		glfwGetWindowPos(m_Window, &x, &y);
		return { static_cast<float>(x), static_cast<float>(y) };
	}

	void WindowsWindow::ProcessEvents()
	{
		glfwPollEvents();
		Input::Update();
	}

	void WindowsWindow::SwapBuffers()
	{
		// m_SwapChain.Present();
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		// m_Specification.VSync = enabled;
		//
		// Application::Get().QueueEvent([&]
		// 	{
		// 		m_SwapChain.SetVSync(m_Specification.VSync);
		// 		m_SwapChain.OnResize(m_Specification.Width, m_Specification.Height);
		// 	});
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Specification.VSync;
	}

	void WindowsWindow::SetResizable(bool resizable) const
	{
		glfwSetWindowAttrib(m_Window, GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
	}

	void WindowsWindow::Maximize()
	{
		glfwMaximizeWindow(m_Window);
	}

	void WindowsWindow::CenterWindow()
	{
		const GLFWvidmode* videmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		int x = videmode->width / 2 - m_Data.Width / 2;
		int y = videmode->height / 2 - m_Data.Height / 2;
		glfwSetWindowPos(m_Window, x, y);
	}

	void WindowsWindow::SetTitle(const std::string& title)
	{
		m_Data.Title = title;
		glfwSetWindowTitle(m_Window, m_Data.Title.c_str());
	}

	// VulkanSwapChain& WindowsWindow::GetSwapChain()
	// {
	// 	return m_SwapChain;
	// }

}
