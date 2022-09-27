#pragma once
#include "CameraController.h"

namespace Akari
{
    class FlyingFPSCamera : public CameraController
    {
    public:
        FlyingFPSCamera(Camera& camera, Math::Vector3 worldUp);

        void Update(float dt) override;

        void SlowMovement(bool enable) { m_FineMovement = enable; }
        void SlowRotation(bool enable) { m_FineRotation = enable; }

        void EnableMomentum(bool enable) { m_Momentum = enable; }

        void SetHeadingPitchAndPosition(float heading, float pitch, const Math::Vector3& position);

    private:
        Math::Vector3 m_WorldUp;
        Math::Vector3 m_WorldNorth;
        Math::Vector3 m_WorldEast;
        float m_HorizontalLookSensitivity;
        float m_VerticalLookSensitivity;
        float m_MoveSpeed;
        float m_StrafeSpeed;
        float m_MouseSensitivityX;
        float m_MouseSensitivityY;

        float m_CurrentHeading;
        float m_CurrentPitch;

        bool m_FineMovement;
        bool m_FineRotation;
        bool m_Momentum;

        float m_LastYaw;
        float m_LastPitch;
        float m_LastForward;
        float m_LastStrafe;
        float m_LastAscent;
    };
}
