// sg_cmdrexp.cpp ____________________________________________________________________________________________________________ 


#include    "segue/tenor/sg_include.h" 
#include    "cove/snip/cv_cmdexec.h" 
#include    "segue/grammar/sg_rexpgrammar.h"
#include    "segue/timbre/sg_timbreparser.h"

#include	<utility>
#include	<tuple>

///_____________________________________________________________________________________________________________________________ 

static Cv_CmdOption     s_RExpIfcOptions[] = 
{     
    { 0, 0,  0},
};

//_____________________________________________________________________________________________________________________________ 

class Sg_RExpCmdProcessor : public Cv_CmdExecutor
{
     

public:
    Sg_RExpCmdProcessor( void)  
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
 
CV_CMD_DEFINE( Sg_RExpCmdProcessor, "rexp", "rexp", s_RExpIfcOptions)

//_____________________________________________________________________________________________________________________________ 

int     Sg_RExpCmdProcessor::Test(void)
{
    using namespace Sg_Timbre;
	using namespace Sg_RExp;   

	std::vector< uint8_t>	memVector;
	bool	res = Cv_Aid::ReadVec( &memVector, "ip.rules");

	RExpDoc   xmlDoc; 

    bool            apiErrCode = 0; // StrParser( str).Match( &xmlDoc);
	Cv_CrateRepos< RExpParserCrate>				synCrate ;
	Cv_CrateConstructor< RExpParserCrate>		synCnstr( &synCrate);
	auto										synElem = synCnstr.FetchElemId( &xmlDoc);
	std::ofstream								ostrm( "b.dot");
	Cv_DotStream								synDotStrm( &ostrm, false); 
	synCrate.OperateAll( [&synDotStrm]( auto k ){
		return k->WriteDot( synDotStrm); 
	});
	synCrate.Clear();
	bool                    t = true; 
    return 0;
}

//_____________________________________________________________________________________________________________________________ 

int     Sg_RExpCmdProcessor::Execute( void)
{
    int     apiErrCode = 0;
    //AC_API_BEGIN() 
    Test();
 
        
    //AC_API_END()
    return apiErrCode;
}

//_____________________________________________________________________________________________________________________________ 

