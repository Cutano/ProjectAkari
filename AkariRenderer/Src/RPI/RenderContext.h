#pragma once

namespace Akari
{
    class Scene;
    class DeltaTime;
    
    struct RenderContext
    {
        std::shared_ptr<Scene> scene;

        DeltaTime* dt;
    };
}
