// sg_cmdxml.cpp ____________________________________________________________________________________________________________ 


#include    "segue/tenor/sg_include.h" 
#include    "cove/barn/cv_cmdexec.h"
#include    "segue/grammar/sg_xmlgrammar.h"
#include    "segue/epigraph/sg_parser.h"

#include	<utility>
#include	<tuple>

//_____________________________________________________________________________________________________________________________ 

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
        std::string		tok;
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
	using namespace Sg_Xml;    

    const char				*str = "<  A a=\"1\" >< B></B></ A>"; 
	StrInStream				strInStrm( str);
	Parser< StrInStream>	parsr( &strInStrm);
	XMLDoc					xmlDoc; 
    bool					apiErrCode = parsr.Match( &xmlDoc);

	Cv_CrateRepos< XmlCrate>				synCrate ;
	Cv_CrateConstructor< XmlCrate>		synCnstr( &synCrate);
	auto										synElem = synCnstr.FetchElemId( &xmlDoc);
	std::ofstream								ostrm( "b.dot");
	Cv_DotStream								synDotStrm( &ostrm, false); 

	synCrate.OperateAll( [&synDotStrm]( auto k, uint32_t ind ){
		return k->WriteDot( Cv_CrateId(), synDotStrm); 
	});
	synCrate.Clear();
	bool                    t = true; 
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

