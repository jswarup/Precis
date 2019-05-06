// cv_easel.h _____________________________________________________________________________________________________________
#pragma once

#include    "cove/silo/cv_craterepos.h" 
#include    "cove/stalks/cv_datacarousal.h" 
#include    "cove/silo/cv_fileflux.h"

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

struct Sg_BaseVita
{    
    Cv_Type< uint32_t>      m_CntActive;
    uint32_t                m_CntEasel;
    bool                    m_StopFlg;

    Sg_BaseVita( void)
        : m_CntActive( 0), m_CntEasel( 0), m_StopFlg( false)
    {}
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

template < typename Vita>
struct Sg_FileReadEasel : public Sg_WorkEasel< Sg_FileReadEasel< Vita>, Vita>
{
    typedef typename Vita::Datagram          Datagram;
    typedef typename Vita::OutPort           OutPort;

    Cv_File         m_InFile;
    OutPort         m_DataPort;
    bool            m_FileClosingFlg;
    uint32_t        m_CharIndex;

    Sg_FileReadEasel( void) 
        : m_FileClosingFlg( false), m_CharIndex( 0)
    {}


    bool    DoInit( Vita *vita)
    {
        if ( !Sg_BaseEasel::DoInit( vita))
            return false;

        if ( !m_InFile.Open( vita->m_InputFile.c_str(), true))
            return false;
        return true;
    }

    bool    IsRunable( void)
    {
        return m_InFile.IsActive();
    }

    void    DoRunStep( void)
    {  
        OutPort::Wharf     wharf( &m_DataPort);
        if ( m_FileClosingFlg && m_InFile.Shut() && wharf.SetClose())
            return;
        uint32_t        szBurst = wharf.Size(); 
        szBurst = wharf.ProbeSzFree( szBurst);
        uint32_t        dInd = 0;
        for ( ; dInd < szBurst;  dInd++)
        {   
            Datagram    *datagram = wharf.AllocFree();
            uint32_t    szFill = m_InFile.Read( datagram->PtrAt( 0), datagram->SzVoid());
            datagram->MarkFill( szFill);
            if ( !szFill || wharf.IsTail()) 
                wharf.Discard( datagram);
            else {
                wharf.Set( dInd, datagram); 
            }
            if ( !szFill)
            {
                m_FileClosingFlg = true;
                break;
            }
        }
        wharf.SetSize( dInd);
        return;
    }
}; 

//_____________________________________________________________________________________________________________________________

template < typename Vita>
struct Sg_FileWriteEasel : public Sg_WorkEasel< Sg_FileWriteEasel< Vita>, Vita>
{
    typedef typename Vita::Datagram          Datagram;
    typedef typename Vita::InPort            InPort;

    Cv_File         m_OutFile;
    InPort          m_DataPort;

    Sg_FileWriteEasel( void) 
    {}

    bool    DoInit( Vita *vita)
    {
        if ( !Sg_BaseEasel::DoInit( vita))
            return false;

        if ( !m_OutFile.Open( vita->m_OutputFile.c_str(), false))
            return false;
        return true;
    }

    bool    IsRunable( void)
    {
        return m_OutFile.IsActive();
    }

    void    DoRunStep( void)
    {   
        InPort::Wharf   wharf( &m_DataPort);
        uint32_t        szBurst = wharf.Size(); 

        if ( !szBurst && wharf.IsClose() && m_OutFile.Shut() && wharf.SetClose())
            return;

        for ( uint32_t i = 0; i < szBurst;  i++)
        {   
            Datagram    *datagram = wharf.Get( i); 
            uint32_t    szWrite = m_OutFile.Write( datagram->PtrAt( 0), datagram->SzFill());  
            wharf.Discard( datagram); 
        }
        return;
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
