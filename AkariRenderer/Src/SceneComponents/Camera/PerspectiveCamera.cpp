#include "pch.h"
#include "PerspectiveCamera.h"

using namespace Math;

namespace Akari
{
    void PerspectiveCamera::UpdateProjMatrix()
    {
        float Y = 1.0f / std::tanf(m_VerticalFOV * 0.5f);
        float X = Y * m_AspectRatio;

        float Q1, Q2;

        // ReverseZ puts far plane at Z=0 and near plane at Z=1.  This is never a bad idea, and it's
        // actually a great idea with F32 depth buffers to redistribute precision more evenly across
        // the entire range.  It requires clearing Z to 0.0f and using a GREATER variant depth test.
        // Some care must also be done to properly reconstruct linear W in a pixel shader from hyperbolic Z.
        if (m_ReverseZ)
        {
            if (m_InfiniteZ)
            {
                Q1 = 0.0f;
                Q2 = m_NearClip;
            }
            else
            {
                Q1 = m_NearClip / (m_FarClip - m_NearClip);
                Q2 = Q1 * m_FarClip;
            }
        }
        else
        {
            if (m_InfiniteZ)
            {
                Q1 = -1.0f;
                Q2 = -m_NearClip;
            }
            else
            {
                Q1 = m_FarClip / (m_NearClip - m_FarClip);
                Q2 = Q1 * m_NearClip;
            }
        }

        SetProjMatrix(Matrix4(
            Vector4(X, 0.0f, 0.0f, 0.0f),
            Vector4(0.0f, Y, 0.0f, 0.0f),
            Vector4(0.0f, 0.0f, Q1, -1.0f),
            Vector4(0.0f, 0.0f, Q2, 0.0f)
        ));
    }
}
