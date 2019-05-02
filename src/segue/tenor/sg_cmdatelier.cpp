// sg_cmdatelier.cpp ____________________________________________________________________________________________________________ 


#include    "trellis/tenor/tr_include.h" 
#include    "cove/snip/cv_cmdexec.h" 
#include    "cove/stalks/cv_datacarousal.h"  
#include    "cove/silo/cv_craterepos.h" 
#include    "cove/silo/cv_fileflux.h" 
#include    "cove/silo/cv_freestore.h" 

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
        if ("-i" == key)
        {
            m_InputFile = arg;
            return true;
        }
        if ("-o" == key)
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

struct Sg_FileReadEasel;
struct Sg_BaseEasel;
struct Sg_AtelierEasel;

typedef Cv_Crate< Sg_FileReadEasel, Sg_AtelierEasel, Sg_BaseEasel>         Sg_AtelierCrate;


//_____________________________________________________________________________________________________________________________

template < typename Datagram, typename SourcePort = std::true_type>
struct Sg_DataPort
{
    typedef Cv_FreeStore< Datagram, uint16_t, 4096> DataStore;
    typedef Cv_FreeCache< 128, DataStore>           DataCache;

    typedef Cv_DataCreek< Datagram *>               Dock;

    Dock                    m_Dock; 
    DataStore               m_DataStore;
    DataCache               m_DataCache; 
    
    Sg_DataPort( void) 
        : m_DataStore( 0x7), m_DataCache( &m_DataStore)
    {}
};

template < typename Datagram>
struct Sg_DataPort< Datagram, std::false_type>
{
    typedef Cv_FreeStore< Datagram, uint16_t, 4096> DataStore;
    typedef Cv_FreeCache< 128, DataStore>           DataCache;

    typedef Cv_DataDock<Datagram>                   Dock;

    Dock                    m_Dock;  
    DataCache               m_DataCache; 

    Sg_DataPort( void)  
        : m_DataCache( NULL)
    {}
};

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
    typedef Cv_FreeStore< Datagram, uint16_t, 4096> DataStore;
    typedef Cv_FreeCache< 128, DataStore>           DataCache;

    typedef Cv_DataCreek<Datagram *>                  Dock;
    
    Xd_InFile               m_InFile;
    Sg_DataPort< Datagram>  m_DataPort;

    Sg_FileReadEasel( void) 
    {}

    Dock    *GetDock( void) { return &m_DataPort.m_Dock; }

    bool    DoInit( Sg_EaselVita *vita)
    {
        if ( !Sg_BaseEasel::DoInit( vita))
            return false;

        if ( !m_InFile.Open( vita->m_InputFile.c_str()))
            return false;
        return true;
    }
    
    bool    IsRunable( void)
    {
        return m_InFile.IsActive();
    }

    void    DoRunStep( void)
    {  
         
        Dock::Wharf     wharf( &m_DataPort.m_Dock);
        uint32_t        szBurst = wharf.Size();
        szBurst = m_DataPort.m_DataCache.ProbeSzFree( szBurst);
        for ( uint32_t i = 0; i < szBurst;  i++)
        {   
            Datagram    *datagram = m_DataPort.m_DataCache.AllocFree();
            uint32_t    szFill = m_InFile.Read( datagram->At( 0), datagram->SzVoid());
            datagram->MarkFill( szFill);
            if ( !szFill || wharf.m_Tail) 
                m_DataPort.m_DataCache.Discard( datagram);
            else
                wharf.Set( i, datagram);
            if ( !szFill &&  m_InFile.Shut())
                break;
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

