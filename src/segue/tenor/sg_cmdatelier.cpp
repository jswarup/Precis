// sg_cmdatelier.cpp ____________________________________________________________________________________________________________ 


#include    "trellis/tenor/tr_include.h" 
#include    "cove/snip/cv_cmdexec.h"  
#include    "cove/stalks/cv_workeasel.h"  
#include    "cove/stalks/cv_fileeasel.h"
#include    "segue/tremolo/sg_ateliereasel.h" 


#include    "segue/grammar/sg_rexpgrammar.h"
#include    "segue/timbre/sg_timbreparser.h"
#include    "segue/tremolo/sg_fsaelemcnstr.h"
#include    "segue/tremolo/sg_dfastate.h"

#include    <utility>
#include    <tuple>


//_____________________________________________________________________________________________________________________________

struct Sg_EaselVita : public Sg_BaseVita
{
    typedef Cv_Array< uint8_t, 4096>                Datagram; 
    typedef Sg_DataSink< Datagram, 64, 1024, 1024>  OutPort; 
    typedef Sg_DataSource< OutPort>                 InPort;
    
    typedef Cv_Array< Sg_MatchData, 1024>           TokenGram;
    typedef Sg_DataSink< TokenGram, 64, 1024, 1024> OutTokPort; 
    typedef Sg_DataSource< OutTokPort>              InTokPort;

    std::string                                     m_ImgFile;
    std::string                                     m_InputFile;
    std::string                                     m_OutputFile; 
    std::string                                     m_TokenLogFile;
    bool                                            m_InputLoopFlg;
    Sg_DfaBlossomAtelier                            *m_Atelier;
    std::vector< uint8_t>                           m_MemArr; 

    Sg_EaselVita( void)
        : m_InputLoopFlg( false), m_Atelier( NULL)
    {}


    bool    DoInit( void)
    {
        if ( m_ImgFile.size())
            m_Atelier = new Sg_DfaBlossomAtelier(  &m_MemArr[ 0]); 
        return true;
    }
};

//_____________________________________________________________________________________________________________________________ 

static Cv_CmdOption     s_AtelierIfcOptions[] = 
{
    { "-idata", "<input>", 0},
    { "-iimg", "<input>", 0},
    { "-oout", "<output>", 0},  
    { "-otok", "<token>", 0},   
    { "-dimg", "<dotfile>", 0},  
    { "-ca", "<value>", "atelier-count"}, 
    { "-loop", 0, 0},
    { 0, 0,  0}
};

//_____________________________________________________________________________________________________________________________ 

class Sg_AtelierCmdProcessor : public Cv_CmdExecutor, public Sg_EaselVita
{ 
    std::string             m_DotFile;
    uint32_t                m_AtelierSz;

public:
    Sg_AtelierCmdProcessor( void)  
        : m_AtelierSz( 1)
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
        if ( "-dimg" == key)
        {
            m_DotFile = arg;
            return true;
        }

        if ( "-idata" == key)
        {
            m_InputFile = arg;
            return true;
        }
        if ( "-oout" == key)
        {
            m_OutputFile = arg;
            return true;
        } 
        if ( "-otok" == key)
        {
            m_TokenLogFile = arg;
            return true;
        } 
        if ( "-ca" == key)
        { 
            char      *prs = NULL;
            m_AtelierSz = strtoul( arg.c_str(), &prs, 10);
            return true;
        } 
        if ( "-loop" == key)
        {
            m_InputLoopFlg = true;
            return true;
        }
        return false;
    }
};


//_____________________________________________________________________________________________________________________________ 
 
CV_CMD_DEFINE( Sg_AtelierCmdProcessor, "atelier", "atelier", s_AtelierIfcOptions) 

//_____________________________________________________________________________________________________________________________
  
struct Sg_ReposEasel;
struct Sg_FileReadAtelierEasel;

typedef Cv_Crate< Sg_AtelierEasel<Sg_EaselVita, Sg_DfaBlossomAtelier>, Sg_TokenLogEasel< Sg_EaselVita>, Sg_FileWriteEasel< Sg_EaselVita>, 
                                                Sg_FileReadAtelierEasel, Sg_ReposEasel, Sg_BaseEasel< Sg_EaselVita> >         Sg_AtelierCrate;

//_____________________________________________________________________________________________________________________________

struct Sg_ReposEasel : public  Sg_MonitorEasel< Sg_ReposEasel, Sg_AtelierCrate, Sg_EaselVita>
{
    typedef Sg_MonitorEasel< Sg_ReposEasel, Sg_AtelierCrate, Sg_EaselVita>     Base;

    typedef Cv_EaselStats   Stats; 

    Sg_ReposEasel( void) 
    {} 

    bool    IsRunable( void)
    {
        return Base::IsRunable();
    }
    
     
};

//_____________________________________________________________________________________________________________________________

struct Sg_FileReadAtelierEasel : public  Sg_FileReadEasel< Sg_FileReadAtelierEasel, Sg_EaselVita>
{
    typedef Sg_FileReadEasel< Sg_FileReadAtelierEasel, Sg_EaselVita>    Base;
    
    Sg_DfaBlossomAtelier        *m_DfaBlossomAtelier; 

    Sg_FileReadAtelierEasel( const std::string &name = "FileRead") 
        : Base( name), m_DfaBlossomAtelier( NULL)
    {}

    //_____________________________________________________________________________________________________________________________

    bool    DoInit( Sg_EaselVita *vita)
    {
        if ( !Base::DoInit( vita))
            return false;  

        if ( vita->m_ImgFile.size())  
            m_DfaBlossomAtelier = vita->m_Atelier;  
        return true;
    }
 
    //_____________________________________________________________________________________________________________________________

    void  ProcessDatagram( Datagram *dgram)
    {
        if ( !m_DfaBlossomAtelier)
            return;
        for ( uint32_t i = 0; i < dgram->SzFill(); ++i)
        {
            uint8_t     *chPtr = dgram->PtrAt( i);
            *chPtr = m_DfaBlossomAtelier->ByteCode( *chPtr);
        }
    }

    //_____________________________________________________________________________________________________________________________

};

//_____________________________________________________________________________________________________________________________ 

int     Sg_AtelierCmdProcessor::Execute(void)
{
    if ( m_ImgFile.size()) 
    { 
        bool	                res = Cv_Aid::ReadVec( &m_MemArr, m_ImgFile.c_str()); 
        if ( !res)
        {
            std::cerr << "Not Found : " << m_ImgFile << '\n';
            return -1;
        } 
    }
    if ( m_DotFile.size() && m_ImgFile.size()) 
    {  
        FsaDfaRepos::Blossom    blossom(  &m_MemArr[ 0]);  
        std::ofstream           fsaOStrm( m_DotFile);
        Cv_DotStream			fsaDotStrm( &fsaOStrm, true);  
        auto                    states = blossom.States();
        FsaDfaRepos::Id         rootId = blossom.RootId();
        for ( uint32_t i = 0; i < states.Size(); ++i)
        {
            auto        var = states.VarAt( i); 
            bool t = true;
            if (var)
                var( [&fsaDotStrm]( auto k) { k->DumpDot( fsaDotStrm); });
        }
        bool t = true; 
    }
    std::chrono::time_point<std::chrono::system_clock> startTime = std::chrono::system_clock::now();  
    
    
    Sg_ReposEasel                       reposEasel; 
    Sg_FileReadAtelierEasel             *fileRead = reposEasel.Construct< Sg_FileReadAtelierEasel>();
    if ( m_ImgFile.size())
    { 
        for ( uint32_t q = 0; q < m_AtelierSz; ++q)
        {
            Sg_AtelierEasel< Sg_EaselVita, Sg_DfaBlossomAtelier>      *atelier = reposEasel.Construct< Sg_AtelierEasel<Sg_EaselVita, Sg_DfaBlossomAtelier>>();         
            atelier->m_AtelierEaseld = q;
            atelier->m_AtelierEaseSz = m_AtelierSz;
            atelier->m_InDataPort.Connect( &fileRead->m_DataPort);
            if ( m_TokenLogFile.size())
            {
                Sg_TokenLogEasel< Sg_EaselVita>      *tokenLog = reposEasel.Construct< Sg_TokenLogEasel<Sg_EaselVita>>(); 
                tokenLog->m_InTokPort.Connect( &atelier->m_TokOutPort); 
            }
        }
        
    }
    if ( m_OutputFile.size())
    {
        Sg_FileWriteEasel< Sg_EaselVita>       *fileWrite = reposEasel.Construct< Sg_FileWriteEasel< Sg_EaselVita>>();
        fileWrite->m_DataPort.Connect( &fileRead->m_DataPort);
    }

    bool    res = DoInit();
    if  ( ! res)
        return -1;

    res = reposEasel.DoInit( this);
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

