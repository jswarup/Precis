// cv_easelstats.h _____________________________________________________________________________________________________________
#pragma once

//_____________________________________________________________________________________________________________________________ 

struct  Cv_EaselStats
{   
    Cv_Type< uint64_t>      m_WorkTime;     
    Cv_Type< uint64_t>      m_ChokeSz;    
    
    void    CopyTo( Cv_EaselStats *inStats)
    {      
        inStats->m_WorkTime = m_WorkTime;
        inStats->m_ChokeSz = m_ChokeSz;
        return;
    }
    
    uint64_t    ActiveUS( Cv_EaselStats *prev) { return m_WorkTime.Diff( prev->m_WorkTime); }
   
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
    Stats            *m_Curr;
    Stats            *m_Prev; 
    Stats            *m_Snap;
 
    Cv_EaselStatsAgent( void)
        : m_Curr( NULL), m_Prev( NULL), m_Snap( NULL)
    {}

    void    InitStats( void)
    {
        m_Curr = new Stats();
        m_Prev = new Stats();
        m_Snap = new Stats();
        return;
    }    

    bool                SnapStats( void)
    { 
        m_Snap->CopyTo( m_Prev);
        m_Curr->CopyTo( m_Snap); 
        return true;
    } 

    bool                ResetLastSnap( void)
    {
        if ( !m_Snap)
            return false;
        delete m_Snap;
        m_Snap = new Stats(); 
        return true;
    }

    bool                LogStats( std::ostream &strm)
    {
        m_Snap->LogStats( strm, m_Prev);
        return true;
    }
};

//_____________________________________________________________________________________________________________________________ 
