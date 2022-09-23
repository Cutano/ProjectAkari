#include "pch.h"
#include "Application.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <filesystem>

#include "RHI/Renderer.h"
#include "Layers/LogicLayer.h"
#include "Layers/ImGuiLayer.h"
#include "RHI/Device.h"
#include "RHI/SwapChain.h"
#include "RPI/RenderContext.h"
#include "RPI/RenderPipeline.h"

extern ImGuiContext* GImGui;
namespace Akari {

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		s_Instance = this;

		if (!specification.WorkingDirectory.empty())
			std::filesystem::current_path(specification.WorkingDirectory);

		m_Profiler = new PerformanceProfiler();

		// Renderer::SetConfig(specification.RenderConfig);

		WindowSpecification windowSpec;
		windowSpec.Title = specification.Name;
		windowSpec.Width = specification.WindowWidth;
		windowSpec.Height = specification.WindowHeight;
		windowSpec.Decorated = specification.WindowDecorated;
		windowSpec.Fullscreen = specification.Fullscreen;
		windowSpec.VSync = specification.VSync;
		m_Window = std::unique_ptr<Window>(Window::Create(windowSpec));
		m_Window->Init();
		m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });
		if (specification.StartMaximized)
			m_Window->Maximize();
		else
			m_Window->CenterWindow();
		m_Window->SetResizable(specification.Resizable);

		// Init renderer and execute command queue to compile all shaders
		Renderer::GetInstance().Init();
		// Renderer::WaitAndRender();

		m_ImGuiLayer = std::make_shared<ImGuiLayer>();
		m_LogicLayer = std::make_shared<LogicLayer>();

		m_ImGuiLayer->SetEventCallback([this](Event& e) { OnEvent(e); });
		m_LogicLayer->SetEventCallback([this](Event& e) { OnEvent(e); });
	}

	Application::~Application()
	{
		m_Window->SetEventCallback([](Event& e) {});

		delete m_Profiler;
		m_Profiler = nullptr;
	}

	void Application::Run()
	{
		OnInit();
		
		m_ImGuiLayer->OnAttach();
		m_LogicLayer->OnAttach();

		m_RenderPipeline->SetGuiLayer(m_ImGuiLayer);

		// TODO: Load resource and prepare first frame
		RenderContext context{};
		
		while (m_Running)
		{
			static uint64_t frameCounter = 0;
			// spdlog::info("-- BEGIN FRAME {0}", frameCounter);
			ProcessEvents();

			if (!m_Minimized)
			{
				Timer cpuTimer;
				// Renderer::BeginFrame();
				{
					SCOPE_PERF("Application Layer::OnUpdate");
					context.dt = &m_DeltaTime;
					m_LogicLayer->OnUpdate(m_DeltaTime);
					// TODO: Tick Renderer
					m_RenderPipeline->Render(context);
					// Renderer::GetInstance().OnUpdate(m_DeltaTime);
				}
			
				// Render ImGui on render thread
				Application* app = this;
				// if (m_Specification.EnableImGui)
				// {
				// 	Renderer::Submit([app] { app->RenderImGui(); });
				// 	Renderer::Submit([app] { app->m_Profiler->Clear(); });
				// 	Renderer::Submit([=] { m_ImGuiLayer->End(); });
				// }
				// Renderer::EndFrame();

				// On Render thread
				// m_Window->GetSwapChain().BeginFrame();
				// Renderer::WaitAndRender();
				m_CPUTime = cpuTimer.ElapsedMillis();
				m_Window->SwapBuffers();
			}

			float time = GetTime();
			m_Frametime = time - m_LastFrameTime;
			m_DeltaTime = std::min<float>(m_Frametime, 0.0333f);
			m_LastFrameTime = time;

			//HZ_CORE_INFO("-- END FRAME {0}", frameCounter);
			frameCounter++;
		}
		
		OnShutdown();
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::OnShutdown()
	{
		m_RenderPipeline.reset();

		m_ImGuiLayer->OnDetach();
		m_LogicLayer->OnDetach();

		Renderer::GetInstance().ShutDown();
		
		m_EventCallbacks.clear();
	}

	void Application::ProcessEvents()
	{
		m_Window->ProcessEvents();

		std::scoped_lock lock(m_EventQueueMutex);

		// Process custom event queue
		while (m_EventQueue.size() > 0)
		{
			auto& func = m_EventQueue.front();
			func();
			m_EventQueue.pop();
		}
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return OnWindowResize(e); });
		dispatcher.Dispatch<WindowMinimizeEvent>([this](WindowMinimizeEvent& e) { return OnWindowMinimize(e); });
		dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return OnWindowClose(e); });
		dispatcher.Dispatch<SceneWindowResizeEvent>([this](SceneWindowResizeEvent& e) { return OnSceneWindowResize(e); });

		// for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		// {
		// 	(*--it)->OnEvent(event);
		// 	if (event.Handled)
		// 		break;
		// }
		
		m_ImGuiLayer->OnEvent(event);
		if (event.Handled) return;
		// m_RendererLayer->OnEvent(event);
		// if (event.Handled) return;

		// TODO(Peter): Should these callbacks be called BEFORE the layers recieve events?
		//				We may actually want that since most of these callbacks will be functions REQUIRED in order for the game
		//				to work, and if a layer has already handled the event we may end up with problems
		for (auto& eventCallback : m_EventCallbacks)
		{
			eventCallback(event);

			if (event.Handled)
				break;
		}

	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		const uint32_t width = e.GetWidth(), height = e.GetHeight();
		if (width == 0 || height == 0)
		{
			//m_Minimized = true;
			return false;
		}
		//m_Minimized = false;
		
		Renderer::GetInstance().OnResize(width, height);

		return false;
	}

	bool Application::OnSceneWindowResize(SceneWindowResizeEvent& e)
	{
		const float width = e.GetWidth(), height = e.GetHeight();
		if (width == 0 || height == 0)
		{
			//m_Minimized = true;
			return false;
		}
		//m_Minimized = false;
		
		Renderer::GetInstance().OnSceneResize(width, height);

		return false;
	}

	bool Application::OnWindowMinimize(WindowMinimizeEvent& e)
	{
		m_Minimized = e.IsMinimized();
		return false;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		Close();
		return false; // give other things a chance to react to window close
	}

	float Application::GetTime() const
	{
		return static_cast<float>(glfwGetTime());
	}

}
