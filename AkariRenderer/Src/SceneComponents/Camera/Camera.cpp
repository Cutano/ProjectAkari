#include "pch.h"
#include "Camera.h"

using namespace Math;

namespace Akari
{
    void Camera::SetLookDirection( Vector3 forward, Vector3 up )
    {
        // Given, but ensure normalization
        Scalar forwardLenSq = LengthSquare(forward);
        forward = Select(forward * RecipSqrt(forwardLenSq), -Vector3(kZUnitVector), forwardLenSq < Scalar(0.000001f));

        // Deduce a valid, orthogonal right vector
        Vector3 right = Cross(forward, up);
        Scalar rightLenSq = LengthSquare(right);
        right = Select(right * RecipSqrt(rightLenSq), Quaternion(Vector3(kYUnitVector), -XM_PIDIV2) * forward, rightLenSq < Scalar(0.000001f));

        // Compute actual up vector
        up = Cross(right, forward);

        // Finish constructing basis
        m_Basis = Matrix3(right, up, -forward);
        m_CameraToWorld.SetRotation(Quaternion(m_Basis));
    }

    void Camera::Update()
    {
        m_PreviousViewProjMatrix = m_ViewProjMatrix;

        m_ViewMatrix = Matrix4(~m_CameraToWorld);
        m_ViewProjMatrix = m_ProjMatrix * m_ViewMatrix;
        m_ReprojectMatrix = m_PreviousViewProjMatrix * Invert(GetViewProjMatrix());

        m_FrustumVS = Frustum( m_ProjMatrix );
        m_FrustumWS = m_CameraToWorld * m_FrustumVS;
    }
}
