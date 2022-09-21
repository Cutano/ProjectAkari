#pragma once

namespace Akari
{
    class Scene;
    class SceneNode;
    class Mesh;
    class DeltaTime;
    
    struct RenderContext
    {
        Scene* scene;
        SceneNode* sceneNode;
        Mesh* mesh;

        DeltaTime* dt;
    };
}
