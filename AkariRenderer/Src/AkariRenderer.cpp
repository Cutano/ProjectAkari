#include "pch.h"

#include "Application/Application.h"

class AkariRendererApp : public Akari::Application
{
public:
    AkariRendererApp(const Akari::ApplicationSpecification& specification, std::string_view projectPath)
        : Application(specification), m_ProjectPath(projectPath)
    {

    }

    void OnInit() override
    {
        spdlog::info("Launching Akari Renderer...");
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

    ApplicationSpecification spec;
    spec.Name = "Akari Renderer";
    spec.WindowWidth = 1600;
    spec.WindowHeight = 900;
    spec.StartMaximized = false;
    spec.WindowDecorated = true;
    spec.VSync = true;

    return new AkariRendererApp(spec, projectPath);
}


int main(const int argc, const char** argv)
{
    const auto app = Akari::CreateApplication(argc, argv);

    app->Run();

    app->Close();
    delete app;
    return 0;
}
