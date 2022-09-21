//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#pragma once
#include "Math/Frustum.h"

namespace Akari
{
    class Camera
    {
    public:
        // Call this function once per frame and after you've changed any state.  This
        // regenerates all matrices.  Calling it more or less than once per frame will break
        // temporal effects and cause unpredictable results.
        void Update();

        // Public functions for controlling where the camera is and its orientation
        void SetEyeAtUp(Math::Vector3 eye, Math::Vector3 at, Math::Vector3 up);
        void SetLookDirection(Math::Vector3 forward, Math::Vector3 up);
        void SetRotation(Math::Quaternion basisRotation);
        void SetPosition(Math::Vector3 worldPos);
        void SetTransform(const Math::AffineTransform& xform);
        // void SetTransform(const Math::OrthogonalTransform& xform);

        [[nodiscard]] Math::Quaternion GetRotation() const { return m_CameraToWorld.GetRotation(); }
        [[nodiscard]] Math::Vector3 GetRightVec() const { return m_Basis.GetX(); }
        [[nodiscard]] Math::Vector3 GetUpVec() const { return m_Basis.GetY(); }
        [[nodiscard]] Math::Vector3 GetForwardVec() const { return -m_Basis.GetZ(); }
        [[nodiscard]] Math::Vector3 GetPosition() const { return m_CameraToWorld.GetTranslation(); }

        // Accessors for reading the various matrices and frusta
        [[nodiscard]] const Math::Matrix4& GetViewMatrix() const { return m_ViewMatrix; }
        [[nodiscard]] const Math::Matrix4& GetProjMatrix() const { return m_ProjMatrix; }
        [[nodiscard]] const Math::Matrix4& GetViewProjMatrix() const { return m_ViewProjMatrix; }
        [[nodiscard]] const Math::Matrix4& GetReprojectionMatrix() const { return m_ReprojectMatrix; }
        [[nodiscard]] const Math::Frustum& GetViewSpaceFrustum() const { return m_FrustumVS; }
        [[nodiscard]] const Math::Frustum& GetWorldSpaceFrustum() const { return m_FrustumWS; }

    protected:
        Math::OrthogonalTransform m_CameraToWorld;

        // Redundant data cached for faster lookups.
        Math::Matrix3 m_Basis;

        // Transforms homogeneous coordinates from world space to view space.  In this case, view space is defined as +X is
        // to the right, +Y is up, and -Z is forward.  This has to match what the projection matrix expects, but you might
        // also need to know what the convention is if you work in view space in a shader.
        Math::Matrix4 m_ViewMatrix; // i.e. "World-to-View" matrix

        // The projection matrix transforms view space to clip space.  Once division by W has occurred, the final coordinates
        // can be transformed by the viewport matrix to screen space.  The projection matrix is determined by the screen aspect 
        // and camera field of view.  A projection matrix can also be orthographic.  In that case, field of view would be defined
        // in linear units, not angles.
        Math::Matrix4 m_ProjMatrix; // i.e. "View-to-Projection" matrix

        // A concatenation of the view and projection matrices.
        Math::Matrix4 m_ViewProjMatrix; // i.e.  "World-To-Projection" matrix.

        // The view-projection matrix from the previous frame
        Math::Matrix4 m_PreviousViewProjMatrix;

        // Projects a clip-space coordinate to the previous frame (useful for temporal effects).
        Math::Matrix4 m_ReprojectMatrix;

        Math::Frustum m_FrustumVS; // View-space view frustum
        Math::Frustum m_FrustumWS; // World-space view frustum

        Camera() : m_CameraToWorld(Math::kIdentity), m_Basis(Math::kIdentity)
        {
        }

        void SetProjMatrix(const Math::Matrix4& ProjMat) { m_ProjMatrix = ProjMat; }
    };

    inline void Camera::SetEyeAtUp(Math::Vector3 eye, Math::Vector3 at, Math::Vector3 up)
    {
        SetLookDirection(at - eye, up);
        SetPosition(eye);
    }

    inline void Camera::SetPosition(Math::Vector3 worldPos)
    {
        m_CameraToWorld.SetTranslation(worldPos);
    }

    inline void Camera::SetTransform(const Math::AffineTransform& xform)
    {
        // By using these functions, we rederive an orthogonal transform.
        SetLookDirection(-xform.GetZ(), xform.GetY());
        SetPosition(xform.GetTranslation());
    }

    inline void Camera::SetRotation(Math::Quaternion basisRotation)
    {
        m_CameraToWorld.SetRotation(Normalize(basisRotation));
        m_Basis = Math::Matrix3(m_CameraToWorld.GetRotation());
    }
}
