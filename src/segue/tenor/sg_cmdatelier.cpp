// sg_cmdatelier.cpp ____________________________________________________________________________________________________________ 


#include    "trellis/tenor/tr_include.h" 
#include    "cove/snip/cv_cmdexec.h"  
#include    "cove/stalks/cv_easel.h"  
#include    "segue/tremolo/sg_ateliereasel.h" 


#include    "segue/grammar/sg_rexpgrammar.h"
#include    "segue/timbre/sg_timbreparser.h"
#include    "segue/tremolo/sg_fsaelemcnstr.h"
#include    "segue/tremolo/sg_dfastate.h"

#include    <utility>
#include    <tuple>

///_____________________________________________________________________________________________________________________________ 

static Cv_CmdOption     s_AtelierIfcOptions[] = 
{
    { "-idata", "<input>", 0},
    { "-iimg", "<input>", 0},
    { "-o", "<output>", 0}, 
    { "-r", "<ruleset>", 0}, 
    { 0, 0,  0}
};

//_____________________________________________________________________________________________________________________________ 

class Sg_AtelierCmdProcessor : public Cv_CmdExecutor
{ 
    std::string     m_InputFile;
    std::string     m_ImgFile;
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
        if ( "-iimg" == key)
        {
            m_ImgFile = arg;
            return true;
        }
        if ( "-idata" == key)
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

struct Sg_EaselVita : public Sg_BaseVita, public Sg_AtelierVita
{};

//_____________________________________________________________________________________________________________________________
  
struct Sg_ReposEasel;

typedef Cv_Crate<Sg_AtelierEasel<Sg_EaselVita>, Sg_FileWriteEasel<Sg_EaselVita>, Sg_FileReadEasel< Sg_EaselVita>, Sg_ReposEasel, Sg_BaseEasel<Sg_EaselVita> >         Sg_AtelierCrate;


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

    std::chrono::time_point<std::chrono::system_clock> startTime = std::chrono::system_clock::now(); 
    Sg_EaselVita            vita;
    vita.m_ImgFile = m_ImgFile;
    vita.m_InputFile = m_InputFile;
    vita.m_OutputFile = m_OutputFile;
    vita.m_RuleFile = m_RuleFile;
    
    Sg_ReposEasel                       reposEasel; 
    Sg_FileReadEasel< Sg_EaselVita>     *fileRead = reposEasel.Construct< Sg_FileReadEasel< Sg_EaselVita>>();
    Sg_AtelierEasel< Sg_EaselVita>      *atelier = reposEasel.Construct< Sg_AtelierEasel<Sg_EaselVita>>(); 
    atelier->m_DataPort.Connect( &fileRead->m_DataPort);

    if ( m_OutputFile.size())
    {
        Sg_FileWriteEasel< Sg_EaselVita>       *fileWrite = reposEasel.Construct< Sg_FileWriteEasel< Sg_EaselVita>>();
        fileWrite->m_DataPort.Connect( &fileRead->m_DataPort);
    }
    bool    res = reposEasel.DoInit( &vita);
    if  ( ! res)
        return -1;

    std::chrono::time_point<std::chrono::system_clock> endTime = std::chrono::system_clock::now(); ;

    uint64_t    time = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
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

