// sg_cmdatelier.cpp ____________________________________________________________________________________________________________ 


#include    "trellis/tenor/tr_include.h" 
#include    "cove/snip/cv_cmdexec.h" 
#include    "cove/stalks/cv_datacarousal.h"  
#include    "cove/silo/cv_craterepos.h" 
#include    "cove/silo/cv_fileflux.h" 

#include    <utility>
#include    <tuple>

///_____________________________________________________________________________________________________________________________ 

static Cv_CmdOption     s_AtelierIfcOptions[] = 
{
    { "-i", "<input>", 0},
    { "-o", "<output>", 0}, 
    { 0, 0,  0}
};

//_____________________________________________________________________________________________________________________________ 

class Sg_AtelierCmdProcessor : public Cv_CmdExecutor
{ 
    std::string     m_InputFile;
    std::string     m_OutputFile;

public:
    Sg_AtelierCmdProcessor( void)  
    {}

    int     Execute( void);
    int     Test(void);

    bool    ProcessProgArgs( std::istream &cmdStrm)
    {
        std::string             tok;
        return !( cmdStrm >> tok) && tok.empty();
    }

    bool    ParseArg( const std::string &key, const std::string &arg)
    {
        if ( "-i" == key)
        {
            m_InputFile = arg;
            return true;
        }
        if ( "-o" == key)
        {
            m_OutputFile = arg;
            return true;
        }
        return false;
    }
};


//_____________________________________________________________________________________________________________________________ 
 
CV_CMD_DEFINE( Sg_AtelierCmdProcessor, "atelier", "atelier", s_AtelierIfcOptions) 

//_____________________________________________________________________________________________________________________________

struct Sg_EaselVita
{
    std::string             m_InputFile;
    std::string             m_OutputFile;
    Cv_Type< uint32_t>      m_CntActive;
    uint32_t                m_CntEasel;
    bool                    m_StopFlg;
    
    Sg_EaselVita( void)
        : m_CntActive( 0), m_CntEasel( 0), m_StopFlg( false)
    {}
};

//_____________________________________________________________________________________________________________________________

struct Sg_FileWriteEasel;
struct Sg_FileReadEasel;
struct Sg_BaseEasel;
struct Sg_AtelierEasel;

typedef Cv_Crate< Sg_FileWriteEasel, Sg_FileReadEasel, Sg_AtelierEasel, Sg_BaseEasel>         Sg_AtelierCrate;

//_____________________________________________________________________________________________________________________________

struct Sg_BaseEasel :  public Cv_CrateEntry
{
    Sg_EaselVita    *m_Vita;
    bool            m_DoneFlg;
    
    Sg_BaseEasel( void)
        : m_Vita( NULL), m_DoneFlg( false)
    {}

    bool    DoInit( Sg_EaselVita *vita)
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

template < typename Easel>
struct Sg_WorkEasel : public Sg_BaseEasel
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

struct Sg_FileReadEasel : public Sg_WorkEasel< Sg_FileReadEasel>
{
    typedef Cv_Array< uint8_t, 4096>                Datagram; 
    typedef Sg_DataSink< Datagram, 128, 4096>       OutPort;

    Cv_File                                 m_InFile;
    OutPort                                 m_DataPort;
    bool                                    m_FileClosingFlg;
    uint32_t                                m_CharIndex;

    Sg_FileReadEasel( void) 
        : m_FileClosingFlg( false), m_CharIndex( 0)
    {}
 

    bool    DoInit( Sg_EaselVita *vita)
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
            uint32_t    szFill = m_InFile.Read( datagram->At( 0), datagram->SzVoid());
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

struct Sg_FileWriteEasel : public Sg_WorkEasel< Sg_FileWriteEasel>
{
    typedef Cv_Array< uint8_t, 4096>                        Datagram; 
    typedef Sg_DataSource< Sg_FileReadEasel::OutPort>     InPort;

    Cv_File         m_OutFile;
    InPort          m_DataPort;

    Sg_FileWriteEasel( void) 
    {}
 
    bool    DoInit( Sg_EaselVita *vita)
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

        if ( !szBurst && wharf.IsClose() && m_OutFile.Shut())
            return;

        for ( uint32_t i = 0; i < szBurst;  i++)
        {   
            Datagram    *datagram = wharf.Get( i); 
            uint32_t    szWrite = m_OutFile.Write( datagram->At( 0), datagram->SzFill());  
            wharf.Discard( datagram); 
        }
        return;
    }
}; 
//_____________________________________________________________________________________________________________________________

struct Sg_AtelierEasel : public  Cv_CrateRepos< Sg_AtelierCrate>, public Sg_WorkEasel< Sg_AtelierEasel>     
{
    std::vector< std::thread>   m_Threads; 

    Sg_AtelierEasel( void) 
    {} 

    bool    DoInit( Sg_EaselVita *vita)
    {
        if ( !Sg_BaseEasel::DoInit( vita))
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

int     Sg_AtelierCmdProcessor::Execute(void)
{
    Sg_EaselVita            vita;
    vita.m_InputFile = m_InputFile;
    vita.m_OutputFile = m_OutputFile;

    Sg_AtelierEasel         atelier; 
    Sg_FileReadEasel        *fileRead = atelier.Construct< Sg_FileReadEasel>();
    Sg_FileWriteEasel       *fileWrite = atelier.Construct< Sg_FileWriteEasel>();
    fileWrite->m_DataPort.Connect( &fileRead->m_DataPort);
    
    atelier.DoInit( &vita);
    atelier.DoLaunch();   

    //AC_API_BEGIN() 

/*
    Cv_ProduceNums             producer;  

    Cv_ConsumeNums              consumer; 
    consumer.Hookup( &producer);   

    Cv_ConsumeNums              consumer1; 
    consumer1.Hookup( &producer); 

    Cv_ConsumeNums              consumer2; 
    consumer2.Hookup( &producer);    

    std::thread             t2{ &Cv_ConsumeNums::DoRun, &consumer2};
    std::thread             t{ &Cv_ConsumeNums::DoRun, &consumer};
    std::thread             t1{ &Cv_ConsumeNums::DoRun, &consumer1};

    producer.DoRun();
    t1.join();
    t.join();
    t2.join(); 
*/
    bool                apiErrCode = false; 
    //AC_API_END()
    return apiErrCode;
}
 

//_____________________________________________________________________________________________________________________________ 

