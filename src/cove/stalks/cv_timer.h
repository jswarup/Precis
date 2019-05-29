// cv_atomic.h _____________________________________________________________________________________________________________
#pragma once

#include    "cove/barn/cv_include.h"

//_____________________________________________________________________________________________________________________________

struct Cv_Time
{
    static  uint64_t    Now( void)  { return std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::system_clock::now() -std::chrono::time_point<std::chrono::system_clock>() ).count(); }
};

//_____________________________________________________________________________________________________________________________

template < typename Keeper>
class Cv_StopWatch
{
    Keeper                                              *m_Keeper;
    std::chrono::time_point<std::chrono::system_clock>  m_StartTime;

public:
    Cv_StopWatch( Keeper *keeper)
        : m_Keeper( keeper), m_StartTime( std::chrono::system_clock::now())
    {}

    ~Cv_StopWatch( void)
    {
        auto        endTime = std::chrono::system_clock::now(); 
        uint64_t    elapsed = std::chrono::duration_cast<std::chrono::microseconds>(endTime - m_StartTime).count();
        *m_Keeper += elapsed;
    } 
};

//_____________________________________________________________________________________________________________________________
