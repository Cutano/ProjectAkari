#pragma once

namespace Akari
{
    class Camera;

    class CameraController
    {
    public:
        // Assumes worldUp is not the X basis vector
        CameraController(Camera& camera) : m_TargetCamera(camera) {}

        virtual ~CameraController() = default;
        virtual void Update(float dt) = 0;

        // Helper function
        static void ApplyMomentum(float& oldValue, float& newValue, float deltaTime);

    protected:
        Camera& m_TargetCamera;

    private:
        CameraController& operator=(const CameraController&) { return *this; }
    };
}
