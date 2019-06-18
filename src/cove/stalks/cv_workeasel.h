// cv_easel.h _____________________________________________________________________________________________________________
#pragma once

#include    "cove/silo/cv_craterepos.h" 
#include    "cove/stalks/cv_datacarousal.h" 
#include    "cove/flux/cv_fileflux.h"
#include    "cove/stalks/cv_timer.h"
#include    "cove/stalks/cv_easelstats.h"

//_____________________________________________________________________________________________________________________________

template < typename Vita>
struct Sg_BaseEasel :  public Cv_CrateEntry
{
    std::string             m_Name;
    Vita                    *m_Vita;
    Cv_Type< uint64_t>      m_Worktime;
    bool                    m_DoneFlg;

    Sg_BaseEasel( const std::string &name)
        : m_Name( name), m_Vita( NULL), m_DoneFlg( false)
    {}

    bool    DoInit( Vita *vita)
    {
        m_Vita = vita; 
        return true;
    } 

    bool                DoLaunch( void) { return false; }

    bool                IsRunnable( void) { return true; }

    bool                DoJoin( void) { return true; } 

    bool                SnapStats( void) {  return false; } 
    bool                ResetLastSnap( void) { return false; }
    bool                LogStats( std::ostream &strm) { return false; }
};

//_____________________________________________________________________________________________________________________________

struct Sg_BaseVita
{    

    Cv_Type< uint32_t>      m_CntActive;
    uint32_t                m_CntEasel;
    bool                    m_StopFlg;
    uint32_t                m_UpdateMSec;

    Sg_BaseVita( void)
        : m_CntActive( 0), m_CntEasel( 0), m_StopFlg( false), m_UpdateMSec( 1000)
    {}
};

//_____________________________________________________________________________________________________________________________

template < typename Easel, typename Vita, typename EaselStats = Cv_EaselStats>
struct Sg_WorkEasel : public Sg_BaseEasel< Vita>, Cv_EaselStatsAgent< Easel, EaselStats>
{ 
    typedef Sg_BaseEasel< Vita>                     Base;
    typedef Cv_EaselStatsAgent< Easel, EaselStats>  StatsAgent;
    typedef EaselStats                              Stats; 

    std::thread     m_Thread;

    Sg_WorkEasel( const std::string &name)
        : Base( name) 
    {}

    Easel           *GetEasel( void) { return static_cast< Easel *>( this); } 

    bool    DoInit( Vita *vita)
    {
        if ( !Base::DoInit( vita))
            return false;
 
        return true;
    }

    void            DoStart( void)
    {
        this->m_Vita->m_CntActive.Incr();
        while ( this->m_Vita->m_CntActive.Get() != this->m_Vita->m_CntEasel)
            std::this_thread::yield();
    }
    void            DoStop( void)
    {
        this->m_Vita->m_CntActive.Decr();
        this->m_DoneFlg = true;
    }

    bool            DoLaunch( void)
    {
        m_Thread  = std::thread( &Easel::DoExecute, GetEasel());
        return true;
    }

    void    DoExecute( void)
    {  
        Easel   *easel = GetEasel();

        easel->DoStart();
        while ( easel->IsRunable())
        {
            Cv_StopWatch< Cv_Type< uint64_t>>   stopWatch( &this->m_Worktime);
            uint64_t    val = this->m_Worktime.Get();
            easel->DoRunStep();
        }
        easel->DoStop();
        return;
    }

    bool    DoJoin( void)
    {
        m_Thread.join();
        return true;
    }

    Stats               *CurStats( void)  { return &this->m_Curr; }
    bool                SnapStats( void) {  return StatsAgent::SnapStats(); } 
    bool                ResetLastSnap( void) { return StatsAgent::ResetLastSnap(); }
    bool                LogStats( std::ostream &strm) 
    { 
        strm << this->m_Name << ": ";
        return StatsAgent::LogStats( strm); 
    }
};

//_____________________________________________________________________________________________________________________________

template < typename Monitor, typename Crate, typename Vita, typename EaselStats = Cv_EaselStats>
struct Sg_MonitorEasel : public  Cv_CrateRepos< Crate>, public Sg_WorkEasel< Monitor, Vita, EaselStats>
{ 
    typedef Sg_WorkEasel< Monitor, Vita>    Base;

    std::vector< std::thread>   m_Threads; 
    uint64_t		            m_PrevUpdateUSec;
    uint64_t                    m_CurUSec; 

    Sg_MonitorEasel( const std::string &name = "Monitor") 
        : Base( name), m_PrevUpdateUSec( 0), m_CurUSec( 0)
    {} 

    bool    DoInit( Vita *vita)
    {
        if ( !Base::DoInit( vita))
            return false;

        bool    res = this->OperateAll( [vita]( auto k) { return k->DoInit( vita); });
        if ( !res)
            return false;
        this->m_Vita->m_CntEasel = this->Size();
        return true;
    }

    bool    IsRunable( void)
    {
        return ( this->m_Vita->m_CntActive.Get() > 1);
    }

    void    DoRunStep( void)
    {
        std::this_thread::yield();;  
        m_CurUSec = Cv_Time::Now();

        uint64_t    usecsSinceUpdate = ( m_CurUSec - m_PrevUpdateUSec);
        bool        updateFLg = usecsSinceUpdate > ( this->m_Vita->m_UpdateMSec * 1000);
        if ( !updateFLg)
            return;
        SnapStats();
        LogStats( std::cout);
        m_PrevUpdateUSec = m_CurUSec;
        return;
              
    }

    bool    DoLaunch( void)
    {  
        bool    res = this->OperateAll( []( auto k) { return k->DoLaunch(); });
        if ( !res)
            return false;  
        this->DoExecute();
        
        res = this->OperateAll( []( auto k) { return k->DoJoin(); });
        SnapStats();
        LogStats( std::cout);
        return res;
    }

    bool        SnapStats( void) 
    {   
        bool    runFlg = !this->m_DoneFlg; 
        bool    res = true;
        if ( !runFlg)       
            res = this->OperateAll( []( auto k) { return k->ResetLastSnap(); }); 
        res = this->OperateAll( []( auto k) { return k->SnapStats(); }); 
        return res; 
    }

    bool        LogStats( std::ostream &strm)
    { 
        bool    res = this->OperateAll( [&strm]( auto k) {  
                    bool    res =  k->LogStats( strm); 
                    strm << '\n'; 
                return res; 
            }); 
        strm << '\n';
        strm.flush();  
        return res;  
    }
};




//_____________________________________________________________________________________________________________________________
