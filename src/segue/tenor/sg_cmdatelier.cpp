// sg_cmdatelier.cpp ____________________________________________________________________________________________________________ 


#include    "trellis/tenor/tr_include.h" 
#include    "cove/snip/cv_cmdexec.h"  
#include    "cove/stalks/cv_easel.h"  
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
    typedef Cv_Array< uint8_t, 4096>                Datagram; 
    typedef Sg_DataSink< Datagram, 128, 4096>       OutPort; 
    typedef Sg_DataSource< OutPort>                 InPort;
    
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
struct Sg_AtelierEasel;

typedef Cv_Crate< Sg_FileWriteEasel, Sg_FileReadEasel, Sg_AtelierEasel, Sg_BaseEasel<Sg_EaselVita> >         Sg_AtelierCrate;


//_____________________________________________________________________________________________________________________________

struct Sg_FileReadEasel : public Sg_WorkEasel< Sg_FileReadEasel, Sg_EaselVita>
{
    typedef Sg_EaselVita::Datagram          Datagram;
    typedef Sg_EaselVita::OutPort           OutPort;
    
    Cv_File         m_InFile;
    OutPort         m_DataPort;
    bool            m_FileClosingFlg;
    uint32_t        m_CharIndex;

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

struct Sg_FileWriteEasel : public Sg_WorkEasel< Sg_FileWriteEasel, Sg_EaselVita>
{
    typedef Sg_EaselVita::Datagram          Datagram;
    typedef Sg_EaselVita::InPort            InPort;


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

struct Sg_AtelierEasel : public  Sg_MonitorEasel< Sg_AtelierEasel, Sg_EaselVita, Sg_AtelierCrate>
{
    typedef Sg_MonitorEasel< Sg_AtelierEasel, Sg_EaselVita, Sg_AtelierCrate>     Base;

    Sg_AtelierEasel( void) 
    {} 

    bool    IsRunable( void)
    {
        return Base::IsRunable();
    }
    
    void    DoRunStep( void)
    {
        std::this_thread::yield();;        
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

