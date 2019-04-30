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
    std::string     m_InputFile;
    std::string     m_OutputFile;
};

//_____________________________________________________________________________________________________________________________

struct Sg_FileReadEasel;
struct Sg_BaseEasel;
struct Sg_AtelierEasel;

typedef Cv_Crate< Sg_FileReadEasel, Sg_AtelierEasel, Sg_BaseEasel>         Sg_AtelierCrate;

//_____________________________________________________________________________________________________________________________

struct Sg_BaseEasel :  public Cv_CrateEntry
{

    bool    DoInit( Sg_EaselVita *vita)
    {
        return true;
    } 

    
};

//_____________________________________________________________________________________________________________________________

template < typename Easel>
struct Sg_WorkEasel
{ 
    Easel   *GetEasel( void) { return static_cast< Easel *>( this); } 

    bool    Initialize( Sg_EaselVita *vita)
    {
        return GetEasel()->DoInit( vita);
    }

    void    DoRun( void)
    {  
        typedef typename Easel::Dock           Dock;
        typedef typename Dock::Wharf           Wharf;
        while( true)
        {
            Easel       *thisEasel = GetEasel();
            Wharf       wharf( thisEasel->GetDock());
            if ( !thisEasel->DoSprint( &wharf))
                break;
        } 
        return;
    }
};

//_____________________________________________________________________________________________________________________________

struct Sg_FileReadEasel : public Sg_WorkEasel< Sg_FileReadEasel>, public Sg_BaseEasel
{
    typedef Cv_DataCreek<uint64_t>      Dock;
    
    Dock            m_Dock; 
    Xd_InFileFlux   m_InFileFLux;


    Sg_FileReadEasel( void) 
    {}

    Dock    *GetDock( void) { return &m_Dock; }

    bool    DoInit( Sg_EaselVita *vita)
    {
        if ( !m_InFileFLux.Open( vita->m_InputFile.c_str()))
            return false;
        return true;
    }

    void    DoRun( void)
    { 
         
        while( true)
        {
            Dock::Wharf     wharf( &m_Dock);
            for ( uint32_t i = 0; i < wharf.Size();  i++)
            {

            }
        }  
        return;
    }
}; 
 

//_____________________________________________________________________________________________________________________________

struct Sg_AtelierEasel : public  Cv_CrateRepos< Sg_AtelierCrate>, public Sg_BaseEasel     
{
    void    DoRun( void)
    { 
        while( true)
        {
        }  
        return;
    }
};

//_____________________________________________________________________________________________________________________________ 

int     Sg_AtelierCmdProcessor::Execute(void)
{
    Sg_AtelierEasel         atelier; 
    Sg_FileReadEasel        *fileRead = atelier.Construct< Sg_FileReadEasel>();
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

