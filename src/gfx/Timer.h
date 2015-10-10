/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <SDL2/SDL.h>
class Timer
{
public:
    Timer ( void )
    {
        m_StartTicks = 0;
        m_PausedTicks = 0;
        m_LastCheckedTicks = 0;
        m_Paused = true;
        m_Started = false;
        TicksPerSec = SDL_GetPerformanceFrequency();
    }
    ~Timer ( void ) { }

    void Start ( void )
    {
        m_Started = true;
        m_Paused = false;
        m_StartTicks = SDL_GetPerformanceCounter();
        m_LastCheckedTicks = SDL_GetPerformanceCounter();;
    }
    void Stop ( void )
    {
        m_StartTicks = 0;
        m_PausedTicks = 0;
        m_Paused = true;
        m_Started = false;
    }
    void Pause()
    {
        if ( !m_Paused && m_Started )
        {
            m_Paused = true;
            m_PausedTicks = SDL_GetPerformanceCounter() - m_StartTicks;
        }
    }
    void UnPause()
    {
        if ( m_Paused )
        {
            m_Paused = false;
            m_StartTicks = SDL_GetPerformanceCounter() - m_PausedTicks;
            m_PausedTicks = 0;
        }
    }
    double Restart()
    {
        double elapsedTicks = Time();
        Start();
        return elapsedTicks;
    }
    double Time() const
    {
        if ( m_Started )
        {
            if ( m_Paused )
            {
                return static_cast<double> ( m_PausedTicks );
            }
            else
            {
                return ( SDL_GetPerformanceCounter() - m_StartTicks ) / ( double ) TicksPerSec;
            }
        }
        return 0;
    }
    double DeltaTime()
    {
        double dt = ( SDL_GetPerformanceCounter() - m_LastCheckedTicks ) / ( double ) TicksPerSec;
        m_LastCheckedTicks = SDL_GetPerformanceCounter();
        return dt;
    }

private:
    Uint64 m_StartTicks, m_PausedTicks, m_LastCheckedTicks;
    Uint64 TicksPerSec;
    bool m_Started, m_Paused;
};
