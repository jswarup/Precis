// sg_cmdrexp.cpp ____________________________________________________________________________________________________________ 


#include    "segue/tenor/sg_include.h" 
#include    "cove/snip/cv_cmdexec.h" 
#include    "segue/grammar/sg_rexpgrammar.h"
#include    "segue/timbre/sg_timbreparser.h"
#include    "segue/tremolo/sg_autom.h"

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

using namespace Sg_Timbre;
using namespace Sg_RExp; 


class TestX
{
    int     i;

public:
    TestX( void)
    {
        i = 10;
    }

    void    Dump( std::ostream &ostr)
    {
        return;
    } 

    friend std::ostream &operator<<( std::ostream &ostr, const RExpQuanta &TestX)
    {
        return ostr;
    } 
};

template < typename TimbreShard>
auto ProcessMatch( TimbreShard *shard, int k = 0) ->   decltype( std::declval<TimbreShard>().Dump( std::declval<std::ostream>()))
{         

    return ;
}
 

template < typename TimbreShard>
auto ProcessMatch( TimbreShard *shard, ...) -> void
{         

    return ;
}

//_____________________________________________________________________________________________________________________________ 

int     Sg_RExpCmdProcessor::Test(void)
{ 
    RExpQuanta                   tx;
    Cv_TypeEngage::Dump( &tx, std::cout, 0);
	StrInStream			    memVector;
	bool	                res = Cv_Aid::ReadVec( &memVector, "ip.rules"); 
	Parser< StrInStream>	parser( &memVector);  
    parser.SetLogStream( &std::cout);
    RExpRepos				rexpCrate;
    RExpDoc					rexpDoc; 
    RExpDoc::XAct           xact( &rexpCrate);
    parser.SetLogStream( &std::cout);
    bool					apiErrCode = parser.Match( &rexpDoc, &xact);

    AutomCnstrRepos         automRepos(  &rexpCrate);
    automRepos.Process();
    std::ofstream           rexpOStrm( "a.dot");
    Cv_DotStream			rexpDotStrm( &rexpOStrm, false); 
    for ( uint32_t i = 1; i < automRepos.m_Elems.size(); ++i)
    {
        auto    si = automRepos.m_Elems[ i];
        si->WriteDot( rexpDotStrm); 
    }
    RExpRepos				                synCrate;
	Cv_CrateConstructor< RExpCrate>		    synCnstr( &synCrate);
	auto								    synElem = synCnstr.FetchElemId( &rexpDoc);
	std::ofstream							ostrm( "b.dot");
	Cv_DotStream						    synDotStrm( &ostrm, false); 
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
	/*
	StrInStream				strInstrm( "100");
	Parser< StrInStream>	parser( &strInstrm);

	auto					lmbda = []( auto ctxt) {
		std::cout << ctxt.MatchStr() << "\n";
		return true;  
	};
	auto				iprs = ParseInt< >()[ lmbda];

	bool					a1 = parser.Match( &iprs);      
	*/
    //AC_API_END()
    return apiErrCode;
}

//_____________________________________________________________________________________________________________________________ 

