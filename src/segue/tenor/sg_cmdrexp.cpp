// sg_cmdrexp.cpp ____________________________________________________________________________________________________________ 


#include    "segue/tenor/sg_include.h" 
#include    "cove/snip/cv_cmdexec.h" 
#include    "segue/grammar/sg_rexpgrammar.h"
#include    "segue/timbre/sg_timbreparser.h"
#include    "segue/tremolo/sg_fsaelemcnstr.h"
#include    "segue/tremolo/sg_dfastate.h"
#include    "segue/timbre/sg_partition.h" 

#include	<utility>
#include	<tuple>

///_____________________________________________________________________________________________________________________________ 

static Cv_CmdOption     s_RExpIfcOptions[] = 
{         
    { "-i",     "<input>",  "input rule-file"},
    { "-d",     0 ,         "debug"},
    { "-oelem",  "<dot>",    0},
    { "-odfa",  "<dot>",    0},
    { "-o",     "<output>", 0},
    { 0,        0,          0},
};

//_____________________________________________________________________________________________________________________________ 

class Sg_RExpCmdProcessor : public Cv_CmdExecutor
{ 
    std::string         m_InputFile;
    std::string         m_ElemDotFile;
    std::string         m_DfaDotFile;
    bool                m_DebugFlg;
    std::string         m_OutputFile;

public:
    Sg_RExpCmdProcessor( void)  
        :  m_DebugFlg( false)
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
        if ("-i" == key)
        {
            m_InputFile = arg;
            return true;
        }
        if ("-d" == key)
        {
            m_DebugFlg = true;
            return true;
        }  
        if ("-oelem" == key)
        {
            m_ElemDotFile = arg;
            return true;
        }  
        if ("-odfa" == key)
        {
            m_DfaDotFile = arg;
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
    if ( !m_InputFile.size())
        return 0;

	StrInStream			    memVector;
	bool	                res = Cv_Aid::ReadVec( &memVector, m_InputFile.c_str()); 
	Parser< StrInStream>	parser( &memVector);  
    if ( m_DebugFlg)
        parser.SetLogStream( &std::cout);
    RExpRepos				rexpRepos;
    RExpDoc					rexpDoc; 
    RExpDoc::XAct           xact( &rexpRepos); 
    bool					apiErrCode = parser.Match( &rexpDoc, &xact);

    FsaElemRepos            automRepos;
    FsaElemReposCnstr       automReposCnstr(  &rexpRepos, &automRepos);
    automReposCnstr.Process();  
    if ( m_ElemDotFile.size())
    {
        std::ofstream       fsaOStrm( m_ElemDotFile);
        Cv_DotStream	    fsaDotStrm( &fsaOStrm, true);  
        automRepos.WriteDot( fsaDotStrm);
    }
    FsaRepos                dfaRepos;
    FsaDfaCnstr             dfaCnstr( &automRepos, &dfaRepos);
    dfaCnstr.SubsetConstruction();
    if ( m_DfaDotFile.size())
    {
        std::ofstream           fsaOStrm( m_DfaDotFile);
        Cv_DotStream			fsaDotStrm( &fsaOStrm, true);  
        dfaRepos.WriteDot( fsaDotStrm);
    }
     
/*
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
*/
    return 0;
}

//_____________________________________________________________________________________________________________________________ 

int     Sg_RExpCmdProcessor::Execute( void)
{
    Sg_Bitset< 7> a;
    Sg_Bitset< 64> b;
    Sg_Bitset< 194> c;

    a.Set( 4, true);
    bool    a1 = a.Get( 4);

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

