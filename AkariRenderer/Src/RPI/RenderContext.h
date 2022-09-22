#pragma once

namespace Akari
{
    class Model;
    class ModelNode;
    class Mesh;
    class Camera;
    class DeltaTime;
    
    struct RenderContext
    {
        Model* scene;
        ModelNode* sceneNode;
        Mesh* mesh;

        Camera* camera;

        DeltaTime* dt;
    };
}
