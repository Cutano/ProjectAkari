#pragma once

namespace Akari {

    class DeltaTime
    {
    public:
        DeltaTime() = default;
        DeltaTime(float time);

        [[nodiscard]] float GetSeconds() const { return m_Time; }
        [[nodiscard]] float GetMilliseconds() const { return m_Time * 1000.0f; }

        operator float() const { return m_Time; }
    private:
        float m_Time = 0.0f;
    };

}