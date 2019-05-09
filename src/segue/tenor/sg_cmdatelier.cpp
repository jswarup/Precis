// sg_cmdatelier.cpp ____________________________________________________________________________________________________________ 


#include    "trellis/tenor/tr_include.h" 
#include    "cove/snip/cv_cmdexec.h"  
#include    "cove/stalks/cv_easel.h"  
#include    "segue/atelier/sg_atelier.h" 


#include    "segue/grammar/sg_rexpgrammar.h"
#include    "segue/timbre/sg_timbreparser.h"
#include    "segue/tremolo/sg_fsaelemcnstr.h"
#include    "segue/tremolo/sg_dfastate.h"

#include    <utility>
#include    <tuple>

///_____________________________________________________________________________________________________________________________ 

static Cv_CmdOption     s_AtelierIfcOptions[] = 
{
    { "-i", "<input>", 0},
    { "-o", "<output>", 0}, 
    { "-r", "<ruleset>", 0}, 
    { 0, 0,  0}
};

//_____________________________________________________________________________________________________________________________ 

class Sg_AtelierCmdProcessor : public Cv_CmdExecutor
{ 
    std::string     m_InputFile;
    std::string     m_OutputFile;
    std::string     m_RuleFile;

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
        if ( "-r" == key)
        {
            m_RuleFile = arg;
            return true;
        }
        return false;
    }
};


//_____________________________________________________________________________________________________________________________ 
 
CV_CMD_DEFINE( Sg_AtelierCmdProcessor, "atelier", "atelier", s_AtelierIfcOptions) 

//_____________________________________________________________________________________________________________________________

struct Sg_EaselVita : public Sg_BaseVita
{    
    typedef Cv_Array< uint8_t, 256>                 Datagram; 
    typedef Sg_DataSink< Datagram, 64, 4096>        OutPort; 
    typedef Sg_DataSource< OutPort>                 InPort;
    
    std::string             m_InputFile;
    std::string             m_OutputFile;
    std::string             m_RuleFile;
    
    Sg_EaselVita( void)
    {}
};

//_____________________________________________________________________________________________________________________________
 
struct Sg_AtelierEasel; 
struct Sg_ReposEasel;

typedef Cv_Crate<Sg_AtelierEasel, Sg_FileWriteEasel<Sg_EaselVita>, Sg_FileReadEasel< Sg_EaselVita>, Sg_ReposEasel, Sg_BaseEasel<Sg_EaselVita> >         Sg_AtelierCrate;

//_____________________________________________________________________________________________________________________________

struct Sg_AtelierEasel : public Sg_WorkEasel< Sg_AtelierEasel, Sg_EaselVita>
{
    typedef Sg_EaselVita::Datagram          Datagram;
    typedef Sg_EaselVita::InPort            InPort;
    typedef typename InPort::Wharf          Wharf;
 
    InPort          m_DataPort;
    FsaDfaRepos     m_DfaRepos;
    Sg_Bulwark      m_Bastion;
    bool            m_CloseFlg;

    Sg_AtelierEasel( void) 
        : m_CloseFlg( false)
    {}

    bool    DoInit( Sg_EaselVita *vita)
    {
        if ( !Sg_BaseEasel::DoInit( vita))
            return false; 
        StrInStream			    memVector;
        bool	                res = Cv_Aid::ReadVec( &memVector, vita->m_RuleFile.c_str()); 
        if ( !res)
        {
            std::cerr << "Not Found : " << vita->m_RuleFile << '\n';
            return false;
        }
        Parser< StrInStream>	parser( &memVector);   
        RExpRepos				rexpRepos;
        RExpDoc					rexpDoc; 
        RExpDoc::XAct           xact( &rexpRepos); 
        bool					apiErrCode = parser.Match( &rexpDoc, &xact); 

        FsaElemRepos            elemRepos;
        FsaElemReposCnstr       automReposCnstr(  &rexpRepos, &elemRepos); 
        automReposCnstr.Process();   
        FsaDfaCnstr             dfaCnstr( &elemRepos, &m_DfaRepos);
        m_Bastion.SetDfaRepos( &m_DfaRepos);
        dfaCnstr.SubsetConstruction();
        m_DfaRepos.m_DistribRepos.Dump( std::cout);
        {
            std::ofstream           fsaOStrm( "a.dot");
            Cv_DotStream			fsaDotStrm( &fsaOStrm, true);  
            m_DfaRepos.WriteDot( fsaDotStrm);
        }
        return true;
    }

    bool    IsRunable( void)
    {
        return !m_CloseFlg;
    }

    void    DoRunStep( void)
    {   
        InPort::Wharf   wharf( &m_DataPort);
        uint32_t        szBurst = wharf.Size(); 

        if ( !szBurst && wharf.IsClose() && (( m_CloseFlg = true)) && wharf.SetClose())
            return;

        uint32_t        dInd = 0;
        for ( ; dInd < szBurst;  dInd++)
        {
            Datagram    *datagram = wharf.Get( dInd); 
            for ( uint32_t k = 0; k < datagram->SzFill(); ++k)
            {
                uint8_t     chr = datagram->At( k);
                 m_Bastion.Play( chr);
            }
        }
        wharf.SetSize( dInd);
        return;
    }
}; 


//_____________________________________________________________________________________________________________________________

struct Sg_ReposEasel : public  Sg_MonitorEasel< Sg_ReposEasel, Sg_EaselVita, Sg_AtelierCrate>
{
    typedef Sg_MonitorEasel< Sg_ReposEasel, Sg_EaselVita, Sg_AtelierCrate>     Base;

    Sg_ReposEasel( void) 
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
    vita.m_RuleFile = m_RuleFile;
    
    Sg_ReposEasel           reposEasel; 
    Sg_FileReadEasel< Sg_EaselVita>        *fileRead = reposEasel.Construct< Sg_FileReadEasel< Sg_EaselVita>>();
    Sg_AtelierEasel         *atelier = reposEasel.Construct< Sg_AtelierEasel>(); 
    atelier->m_DataPort.Connect( &fileRead->m_DataPort);

    Sg_FileWriteEasel< Sg_EaselVita>       *fileWrite = reposEasel.Construct< Sg_FileWriteEasel< Sg_EaselVita>>();
    fileWrite->m_DataPort.Connect( &fileRead->m_DataPort);
    
    reposEasel.DoInit( &vita);
    reposEasel.DoLaunch();   

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

