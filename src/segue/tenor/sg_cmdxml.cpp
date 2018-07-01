// sg_cmdxml.cpp ____________________________________________________________________________________________________________ 


#include    "segue/tenor/sg_include.h" 
#include    "cove/snip/cv_cmdexec.h"
#include    "segue/xml/sg_xmlgrammar.h"
#include    "segue/timbre/sg_timbreparser.h"

#include <utility>
#include  <tuple>

///_____________________________________________________________________________________________________________________________ 

static Cv_CmdOption     s_XmlIfcOptions[] = 
{
     
    { 0, 0,  0},
};

//_____________________________________________________________________________________________________________________________ 

class Sg_XmlCmdProcessor : public Cv_CmdExecutor
{
     

public:
    Sg_XmlCmdProcessor( void)  
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
         
        return false;
    }
};


//_____________________________________________________________________________________________________________________________ 
 
CV_CMD_DEFINE( Sg_XmlCmdProcessor, "xml", "xml", s_XmlIfcOptions)

//_____________________________________________________________________________________________________________________________ 

int     Sg_XmlCmdProcessor::Test(void)
{
    using namespace Sg_Timbre;
    
    const char  *str = "<  A a=\"1\" >< B></B></ A>"; 

    Sg_Xml::XMLDoc   xmlDoc; 
    bool            apiErrCode = StrParser( str).Match( &xmlDoc);
    return 0;
}

//_____________________________________________________________________________________________________________________________ 

int     Sg_XmlCmdProcessor::Execute( void)
{
    int     apiErrCode = 0;
    //AC_API_BEGIN() 
    Test();
 
        
    //AC_API_END()
    return apiErrCode;
}

//_____________________________________________________________________________________________________________________________ 

