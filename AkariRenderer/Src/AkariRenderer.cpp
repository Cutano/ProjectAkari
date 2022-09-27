#include "pch.h"

#include "Application/Application.h"
#include "RenderPipelines/ForwardPipeline.h"

class AkariRendererApp : public Akari::Application
{
public:
    AkariRendererApp(const Akari::ApplicationSpecification& specification, std::string_view projectPath)
        : Application(specification), m_ProjectPath(projectPath)
    {

    }

    void OnInit() override
    {
        spdlog::set_level(spdlog::level::trace);
        spdlog::info("Launching Akari Renderer...");
        
        m_RenderPipeline = std::make_shared<Akari::ForwardPipeline>();
    }

    void OnShutdown() override
    {
        
    }

private:
    std::string m_ProjectPath;
};

Akari::Application* Akari::CreateApplication(const int argc, const char** argv)
{
    std::string_view projectPath;
    if (argc > 1)
        projectPath = argv[1];

    const std::filesystem::path exePath(argv[0]);

    ApplicationSpecification spec;
    spec.Name = "Akari Renderer";
    spec.WindowWidth = 1600;
    spec.WindowHeight = 900;
    spec.StartMaximized = false;
    spec.WindowDecorated = true;
    spec.VSync = true;
    spec.WorkingDirectory = exePath.parent_path().string();

    return new AkariRendererApp(spec, projectPath);
}


int main(const int argc, const char** argv)
{
    const auto app = Akari::CreateApplication(argc, argv);

    app->Run();

    delete app;
    return 0;
}
