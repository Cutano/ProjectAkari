#include "pch.h"
#include "FlyingFPSCamera.h"
#include "Camera.h"
#include "Input/Input.h"

using namespace Math;

namespace Akari
{
    FlyingFPSCamera::FlyingFPSCamera(Camera& camera, Vector3 worldUp) : CameraController(camera)
    {
        m_WorldUp = Normalize(worldUp);
        m_WorldNorth = Normalize(Cross(m_WorldUp, Vector3(kXUnitVector)));
        m_WorldEast = Cross(m_WorldNorth, m_WorldUp);

        m_HorizontalLookSensitivity = 2.0f;
        m_VerticalLookSensitivity = 2.0f;
        m_MoveSpeed = 10.0f;
        m_StrafeSpeed = 10.0f;
        m_MouseSensitivityX = 1.0f;
        m_MouseSensitivityY = 1.0f;

        m_CurrentPitch = Sin(Dot(camera.GetForwardVec(), m_WorldUp));

        Vector3 forward = Normalize(Cross(m_WorldUp, camera.GetRightVec()));
        m_CurrentHeading = ATan2(-Dot(forward, m_WorldEast), Dot(forward, m_WorldNorth));

        m_FineMovement = false;
        m_FineRotation = false;
        m_Momentum = true;

        m_LastYaw = 0.0f;
        m_LastPitch = 0.0f;
        m_LastForward = 0.0f;
        m_LastStrafe = 0.0f;
        m_LastAscent = 0.0f;
    }

    void FlyingFPSCamera::Update(float deltaTime)
    {
        if (Input::IsMouseButtonPressed(MouseButton::Right))
        {
            if (Input::IsKeyPressed(KeyCode::LeftShift))
                m_FineMovement = !m_FineMovement;

            if (Input::IsKeyPressed(KeyCode::LeftControl))
                m_FineRotation = !m_FineRotation;

            float speedScale = (m_FineMovement ? 0.1f : 1.0f);
            float panScale = (m_FineRotation ? 0.5f : 1.0f);

            float yaw = 0;
            float pitch = 0;
            float forward = m_MoveSpeed * speedScale * (
                (Input::IsKeyPressed(KeyCode::W) ? deltaTime : 0.0f) +
                (Input::IsKeyPressed(KeyCode::S) ? -deltaTime : 0.0f)
            );
            float strafe = m_StrafeSpeed * speedScale * (
                (Input::IsKeyPressed(KeyCode::D) ? deltaTime : 0.0f) +
                (Input::IsKeyPressed(KeyCode::A) ? -deltaTime : 0.0f)
            );
            float ascent = m_StrafeSpeed * speedScale * (
                (Input::IsKeyPressed(KeyCode::E) ? deltaTime : 0.0f) +
                (Input::IsKeyPressed(KeyCode::Q) ? -deltaTime : 0.0f)
            );

            if (m_Momentum)
            {
                ApplyMomentum(m_LastYaw, yaw, deltaTime);
                ApplyMomentum(m_LastPitch, pitch, deltaTime);
                ApplyMomentum(m_LastForward, forward, deltaTime);
                ApplyMomentum(m_LastStrafe, strafe, deltaTime);
                ApplyMomentum(m_LastAscent, ascent, deltaTime);
            }

            // don't apply momentum to mouse inputs
            float mouseX = Input::GetMouseDeltaX(), mouseY = -Input::GetMouseDeltaY();
            yaw += 1.2f * mouseX * m_MouseSensitivityX;
            pitch += 1.2f * mouseY * m_MouseSensitivityY;

            m_CurrentPitch += pitch;
            m_CurrentPitch = XMMin(XM_PIDIV2, m_CurrentPitch);
            m_CurrentPitch = XMMax(-XM_PIDIV2, m_CurrentPitch);

            m_CurrentHeading -= yaw;
            if (m_CurrentHeading > XM_PI)
                m_CurrentHeading -= XM_2PI;
            else if (m_CurrentHeading <= -XM_PI)
                m_CurrentHeading += XM_2PI;

            Matrix3 orientation = Matrix3(m_WorldEast, m_WorldUp, -m_WorldNorth) * Matrix3::MakeYRotation(
                    m_CurrentHeading)
                * Matrix3::MakeXRotation(m_CurrentPitch);
            Vector3 position = orientation * Vector3(strafe, ascent, -forward) + m_TargetCamera.GetPosition();
            m_TargetCamera.SetTransform(AffineTransform(orientation, position));
        }
        else
        {
            float mouseX = Input::GetMouseDeltaX(), mouseY = -Input::GetMouseDeltaY();
        }


        m_TargetCamera.Update();
    }

    void FlyingFPSCamera::SetHeadingPitchAndPosition(float heading, float pitch, const Vector3& position)
    {
        m_CurrentHeading = heading;
        if (m_CurrentHeading > XM_PI)
            m_CurrentHeading -= XM_2PI;
        else if (m_CurrentHeading <= -XM_PI)
            m_CurrentHeading += XM_2PI;

        m_CurrentPitch = pitch;
        m_CurrentPitch = XMMin(XM_PIDIV2, m_CurrentPitch);
        m_CurrentPitch = XMMax(-XM_PIDIV2, m_CurrentPitch);

        Matrix3 orientation =
            Matrix3(m_WorldEast, m_WorldUp, -m_WorldNorth) *
            Matrix3::MakeYRotation(m_CurrentHeading) *
            Matrix3::MakeXRotation(m_CurrentPitch);

        m_TargetCamera.SetTransform(AffineTransform(orientation, position));
        m_TargetCamera.Update();
    }
}
