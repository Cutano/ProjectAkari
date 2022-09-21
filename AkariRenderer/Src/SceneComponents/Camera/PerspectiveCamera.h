#pragma once
#include "Camera.h"

namespace Akari
{
    class PerspectiveCamera : public Camera
    {
    public:
        PerspectiveCamera();

        // Controls the view-to-projection matrix
        void SetPerspectiveMatrix( float verticalFovRadians, float aspectHeightOverWidth, float nearZClip, float farZClip );
        void SetFOV( float verticalFovInRadians ) { m_VerticalFOV = verticalFovInRadians; UpdateProjMatrix(); }
        void SetAspectRatio( float heightOverWidth ) { m_AspectRatio = heightOverWidth; UpdateProjMatrix(); }
        void SetZRange( float nearZ, float farZ) { m_NearClip = nearZ; m_FarClip = farZ; UpdateProjMatrix(); }
        void ReverseZ( bool enable ) { m_ReverseZ = enable; UpdateProjMatrix(); }

        [[nodiscard]] float GetFOV() const { return m_VerticalFOV; }
        [[nodiscard]] float GetNearClip() const { return m_NearClip; }
        [[nodiscard]] float GetFarClip() const { return m_FarClip; }
        [[nodiscard]] float GetClearDepth() const { return m_ReverseZ ? 0.0f : 1.0f; }
        
    private:
        void UpdateProjMatrix();

        float m_VerticalFOV;	// Field of view angle in radians
        float m_AspectRatio;
        float m_NearClip;
        float m_FarClip;
        bool m_ReverseZ;		// Invert near and far clip distances so that Z=1 at the near plane
        bool m_InfiniteZ;       // Move the far plane to infinity
    };

    inline PerspectiveCamera::PerspectiveCamera() : m_ReverseZ(true), m_InfiniteZ(false)
    {
        SetPerspectiveMatrix( Math::XM_PIDIV4, 9.0f / 16.0f, 1.0f, 1000.0f );
    }

    inline void PerspectiveCamera::SetPerspectiveMatrix( float verticalFovRadians, float aspectHeightOverWidth, float nearZClip, float farZClip )
    {
        m_VerticalFOV = verticalFovRadians;
        m_AspectRatio = aspectHeightOverWidth;
        m_NearClip = nearZClip;
        m_FarClip = farZClip;

        UpdateProjMatrix();

        m_PreviousViewProjMatrix = m_ViewProjMatrix;
    }
}
