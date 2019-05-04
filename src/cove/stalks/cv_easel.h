// cv_easel.h _____________________________________________________________________________________________________________
#pragma once


#include    "cove/silo/cv_craterepos.h" 
#include    "cove/stalks/cv_datacarousal.h" 

//_____________________________________________________________________________________________________________________________

template < typename Vita>
struct Sg_BaseEasel :  public Cv_CrateEntry
{
    Vita    *m_Vita;
    bool            m_DoneFlg;

    Sg_BaseEasel( void)
        : m_Vita( NULL), m_DoneFlg( false)
    {}

    bool    DoInit( Vita *vita)
    {
        m_Vita = vita; 
        return true;
    } 

    bool    DoLaunch( void)
    {
        return false;
    }

    bool    IsRunnable( void)
    {
        return true;
    }

    bool    DoJoin( void)
    {
        return true;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename Easel, typename Vita>
struct Sg_WorkEasel : public Sg_BaseEasel< Vita>
{ 
    std::thread     m_Thread;

    Easel           *GetEasel( void) { return static_cast< Easel *>( this); } 

    void            DoStart( void)
    {
        m_Vita->m_CntActive.Incr();
        while ( m_Vita->m_CntActive.Get() != m_Vita->m_CntEasel)
            std::this_thread::yield();
    }
    void            DoStop( void)
    {
        m_Vita->m_CntActive.Decr();
        m_DoneFlg = true;
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
            easel->DoRunStep();
        easel->DoStop();
        return;
    }

    bool    DoJoin( void)
    {
        m_Thread.join();
        return true;
    }
};


//_____________________________________________________________________________________________________________________________

template < typename Monitor, typename Vita, typename Crate>
struct Sg_MonitorEasel : public  Cv_CrateRepos< Crate>, public Sg_WorkEasel< Monitor, Vita>
{
    std::vector< std::thread>   m_Threads; 

    Sg_MonitorEasel( void) 
    {} 

    bool    DoInit( Vita *vita)
    {
        if ( !Sg_WorkEasel< Monitor, Vita>::DoInit( vita))
            return false;

        bool    res = OperateAll( [vita]( auto k) { return k->DoInit( vita); });
        if ( !res)
            return false;
        m_Vita->m_CntEasel = Size();
        return true;
    }

    bool    IsRunable( void)
    {
        return ( m_Vita->m_CntActive.Get() > 1);
    }

    void    DoRunStep( void)
    {
        std::this_thread::yield();;        
    }

    bool    DoLaunch( void)
    {  
        bool    res = OperateAll( []( auto k) { return k->DoLaunch(); });
        if ( !res)
            return false;  
        DoExecute();

        res = OperateAll( []( auto k) { return k->DoJoin(); });
        return res;
    }

};

//_____________________________________________________________________________________________________________________________
