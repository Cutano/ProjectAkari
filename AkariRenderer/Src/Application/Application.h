#pragma once

#include "Timing/DeltaTime.h"
#include "Timing/Timer.h"
#include "Window/Window.h"
#include "Events/ApplicationEvent.h"

#include <queue>


namespace Akari {
	class Layer;
	
	struct RendererConfig
	{
		uint32_t FramesInFlight = 3;

		bool ComputeEnvironmentMaps = true;

		// Tiering settings
		uint32_t EnvironmentMapResolution = 1024;
		uint32_t IrradianceMapComputeSamples = 512;

		std::string ShaderPackPath;
	};

	struct ApplicationSpecification
	{
		std::string Name = "Hazel";
		uint32_t WindowWidth = 1600, WindowHeight = 900;
		bool WindowDecorated = false;
		bool Fullscreen = false;
		bool VSync = true;
		std::string WorkingDirectory;
		bool StartMaximized = true;
		bool Resizable = true;
		bool EnableImGui = true;
		RendererConfig RenderConfig;
	};
	
	class Application
	{
		using EventCallbackFn = std::function<void(Event&)>;

	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void Run();
		void Close();

		virtual void OnInit() {}
		virtual void OnShutdown();
		virtual void OnUpdate(DeltaTime ts) {}

		virtual void OnEvent(Event& event);

		void AddEventCallback(const EventCallbackFn& eventCallback) { m_EventCallbacks.push_back(eventCallback); }

		void SetShowStats(bool show) { m_ShowStats = show; }

		template<typename Func>
		void QueueEvent(Func&& func)
		{
			m_EventQueue.push(func);
		}

		/// Creates & Dispatches an event either immediately, or adds it to an event queue which will be proccessed at the end of each frame
		template<typename TEvent, bool DispatchImmediately = false, typename... TEventArgs>
		void DispatchEvent(TEventArgs&&... args)
		{
			static_assert(std::is_assignable_v<Event, TEvent>);

			std::shared_ptr<TEvent> event = std::make_shared<TEvent>(std::forward<TEventArgs>(args)...);
			if constexpr (DispatchImmediately)
			{
				OnEvent(*event);
			}
			else
			{
				std::scoped_lock lock(m_EventQueueMutex);
				m_EventQueue.push([event] { Get().OnEvent(*event); });
			}
		}


		Window& GetWindow() { return *m_Window; }
		
		static Application& Get() { return *s_Instance; }

		DeltaTime GetDeltaTime() const { return m_DeltaTime; }
		DeltaTime GetFrametime() const { return m_Frametime; }
		float GetTotalCPUTime() const { return m_CPUTime; }
		float GetTime() const; // TODO: This should be in "Platform"

		const ApplicationSpecification& GetSpecification() const { return m_Specification; }

		PerformanceProfiler* GetPerformanceProfiler() { return m_Profiler; }
	
	private:
		void ProcessEvents();

		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowMinimize(WindowMinimizeEvent& e);
		bool OnWindowClose(WindowCloseEvent& e);
		
	private:
		std::unique_ptr<Window> m_Window;
		ApplicationSpecification m_Specification;
		bool m_Running = true, m_Minimized = false;
		DeltaTime m_Frametime;
		DeltaTime m_DeltaTime;
		PerformanceProfiler* m_Profiler = nullptr; // TODO: Should be null in Dist
		bool m_ShowStats = true;

		std::mutex m_EventQueueMutex;
		std::queue<std::function<void()>> m_EventQueue;
		std::vector<EventCallbackFn> m_EventCallbacks;

		float m_CPUTime = 0.0f;
		float m_LastFrameTime = 0.0f;

		static Application* s_Instance;

	protected:
		std::shared_ptr<Layer> m_ImGuiLayer;
		std::shared_ptr<Layer> m_LogicLayer;
	};

	// Implemented by CLIENT
	Application* CreateApplication(int argc, const char** argv);
}
