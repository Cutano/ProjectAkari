#pragma once

namespace Akari
{
    class Scene;
    class SceneNode;
    class Mesh;
    class Camera;
    class DeltaTime;
    
    struct RenderContext
    {
        Scene* scene;
        SceneNode* sceneNode;
        Mesh* mesh;

        Camera* camera;

        DeltaTime* dt;
    };
}
