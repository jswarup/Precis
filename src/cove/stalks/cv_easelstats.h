// cv_easelstats.h _____________________________________________________________________________________________________________
#pragma once

//_____________________________________________________________________________________________________________________________ 

struct  Cv_EaselStats
{   
    Cv_Type< uint64_t>      m_WorkTime;     
    Cv_Type< uint64_t>      m_ChokeSz;    
     
    
    uint64_t    ActiveUS( Cv_EaselStats *prev) { return m_WorkTime.Get() - prev->m_WorkTime.Get(); }
   
    void    LogStats( std::ostream &strm, Cv_EaselStats *prev)
    {
        strm << "ActiveTime[ " << ActiveUS( prev) << "] ";
        return;
    }
};

//_____________________________________________________________________________________________________________________________ 

template < typename Easel, typename Stats>
struct  Cv_EaselStatsAgent
{
    Stats            m_Curr;
    Stats            m_Prev; 
    Stats            m_Snap;
 
    Cv_EaselStatsAgent( void) 
    {}
 
    bool                SnapStats( void)
    { 
        m_Snap = m_Curr; 
        return true;
    } 

    bool                ResetLastSnap( void)
    { 
        m_Snap = Stats(); 
        return true;
    }

    bool                LogStats( std::ostream &strm)
    {
        m_Snap.LogStats( strm, &m_Prev);
        m_Prev = m_Snap;
        return true;
    }
};

//_____________________________________________________________________________________________________________________________ 
