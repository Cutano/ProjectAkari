#pragma once

#include <chrono>
#include <unordered_map>

namespace Akari {

    class Timer
    {
    public:
        Timer()
        {
            Reset();
        }

        void Reset()
        {
            m_Start = std::chrono::high_resolution_clock::now();
        }

        float Elapsed()
        {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f * 0.001f * 0.001f;
        }

        float ElapsedMillis()
        {
            return Elapsed() * 1000.0f;
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
    };
	
    class ScopedTimer
    {
    public:
        ScopedTimer(const std::string& name)
            : m_Name(name) {}
        ~ScopedTimer()
        {
            float time = m_Timer.ElapsedMillis();
            spdlog::trace("[TIMER] {0} - {1}ms", m_Name, time);
        }
    private:
        std::string m_Name;
        Timer m_Timer;
    };

    class PerformanceProfiler
    {
    public:
        void SetPerFrameTiming(const char* name, float time)
        {
            if (m_PerFrameData.find(name) == m_PerFrameData.end())
                m_PerFrameData[name] = 0.0f;

            m_PerFrameData[name] += time;
        }

        void Clear() { m_PerFrameData.clear(); }

        const std::unordered_map<const char*, float>& GetPerFrameData() const { return m_PerFrameData; }
    private:
        std::unordered_map<const char*, float> m_PerFrameData;
    };

    class ScopePerfTimer
    {
    public:
        ScopePerfTimer(const char* name, PerformanceProfiler* profiler)
            : m_Name(name), m_Profiler(profiler) {}

        ~ScopePerfTimer()
        {
            float time = m_Timer.ElapsedMillis();
            m_Profiler->SetPerFrameTiming(m_Name, time);
        }
    private:
        const char* m_Name;
        PerformanceProfiler* m_Profiler;
        Timer m_Timer;
    };

#define SCOPE_PERF(name)\
ScopePerfTimer timer__LINE__(name, Application::Get().GetPerformanceProfiler());

#define SCOPE_TIMER(name)\
ScopedTimer timer__LINE__(name);
}