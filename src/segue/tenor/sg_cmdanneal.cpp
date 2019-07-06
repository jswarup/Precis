// sg_cmdanneal.cpp ____________________________________________________________________________________________________________ 


#include    "segue/tenor/sg_include.h" 
#include    "cove/barn/cv_cmdexec.h" 
#include    "cove/silo/cv_stash.h"
#include    "segue/grammar/sg_rexpgrammar.h"
#include    "segue/epigraph/sg_parser.h"
#include    "segue/colophon/sg_fsaelemcnstr.h"
#include    "segue/colophon/sg_dfastate.h"
#include    "segue/epigraph/sg_partition.h" 
#include    "segue/colophon/sg_atelier.h"
#include	"cove/flux/cv_cask.h"

#include	<utility>
#include	<tuple>

///_____________________________________________________________________________________________________________________________ 

static Cv_CmdOption     s_AnnealIfcOptions[] = 
{         
    { "-irex",      "<input>",  "input rule-file"},
    { "-ilex",      "<input>",  "input rule-file"},
    { "-g",         0,          "debug"},
    { "-dshard",    "<dot>",    0},
    { "-delem",     "<dot>",    0},
    { "-ddfa",      "<dot>",    0},
    { "-dimg",      "<dot>",    0},
    { "-oimg",      "<image>",    0},
    { "-idata",     "<input>",  0},
    { 0,            0,          0},
};

//_____________________________________________________________________________________________________________________________ 

class Sg_AnealCmdProcessor : public Cv_CmdExecutor
{ 
    std::string         m_LexInputFile;
    std::string         m_RexInputFile;
    std::string         m_DataFile;
    std::string         m_ImgFile;
    std::string         m_ElemDotFile;
    std::string         m_ShardDotFile;
    std::string         m_DfaDotFile;
    std::string         m_ImgDotFile;
    bool                m_DebugFlg;
    std::string         m_OutputFile;

public:
    Sg_AnealCmdProcessor( void)  
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
        if ("-irex" == key)
        {
            m_RexInputFile = arg;
            return true;
        }
        if ("-ilex" == key)
        {
            m_LexInputFile = arg;
            return true;
        }
        if ("-g" == key)
        {
            m_DebugFlg = true;
            return true;
        }  
        if ("-delem" == key)
        {
            m_ElemDotFile = arg;
            return true;
        }  
        if ("-dshard" == key)
        {
            m_ShardDotFile = arg;
            return true;
        }  
        if ("-ddfa" == key)
        {
            m_DfaDotFile = arg;
            return true;
        }
        if ("-dimg" == key)
        {
            m_ImgDotFile = arg;
            return true;
        }
        if ("-o" == key)
        {
            m_OutputFile = arg;
            return true;
        }
        if ("-idata" == key)
        {
            m_DataFile = arg;
            return true;
        }  
        if ("-oimg" == key)
        {
            m_ImgFile = arg;
            return true;
        }
        return false;
    }
};


//_____________________________________________________________________________________________________________________________ 
 
CV_CMD_DEFINE( Sg_AnealCmdProcessor, "anneal", "anneal", s_AnnealIfcOptions)

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

int     Sg_AnealCmdProcessor::Test(void)
{ 
    int32_t	    apiErrCode = 0;
   // typedef Cv_Stash< Sg_CharPartition, 256, 4>     PartitionStash;

  /*  std::cout << PartitionStash::SizeOf( 256) << " ";
    std::cout << PartitionStash::SizeOf( 128) << " ";
    std::cout << PartitionStash::SizeOf( 64) << " ";
    std::cout << PartitionStash::SizeOf( 32) << " ";
    std::cout << PartitionStash::SizeOf( 16) << " ";
    std::cout << PartitionStash::SizeOf( 8) << " ";
    std::cout << PartitionStash::SizeOf( 4) << " " << "\n";
    */
    //RExpQuanta                   tx;
    //Cv_TypeEngage::Dump( &tx, std::cout, 0);
    StrInStream			    memVector;
    bool                    posizFlg = false;
    if ( m_RexInputFile.size())
	{
	    bool	                res = Cv_Aid::ReadVec( memVector.CharVec(), m_RexInputFile.c_str()); 
        if ( !res)
        {
            std::cerr << "Not Found : " << m_RexInputFile << '\n';
            return -1;
        }
    } else if ( m_LexInputFile.size())
    { 
        bool	                res = Cv_Aid::ReadVec( memVector.CharVec(), m_LexInputFile.c_str()); 
        if ( !res)
        {
            std::cerr << "Not Found : " << m_LexInputFile << '\n';
            return -1;
        }
        posizFlg = true;
    } else
        return 0;
    
    
	Parser< StrInStream>	parser( &memVector);  
    if ( m_DebugFlg)
        parser.SetLogStream( &std::cout);
    RExpRepos				rexpRepos;
    rexpRepos.m_PosixFmt = posizFlg;
    
    RExpDoc					rexpDoc; 
    RExpDoc::XAct           xact( &rexpRepos); 
    apiErrCode = parser.Match( &rexpDoc, &xact) ? 0 : -1;
    
    if ( m_ShardDotFile.size())
    {
        std::ofstream							ostrm( m_ShardDotFile);
        Cv_DotStream						    synDotStrm( &ostrm, false); 
        rexpRepos.OperateAll( [&synDotStrm]( auto k, uint32_t ind ){
            return k->WriteDot( Cv_CrateId( ind, RExpCrate::TypeOf( k)), synDotStrm); 
        });
    } 
    //std::cout << rexpRepos.m_Base.ToString() << '\n';

    if ( !m_ElemDotFile.size() && !m_DfaDotFile.size() && !m_ImgFile.size() && !m_DataFile.size())
        return apiErrCode;

    FsaElemRepos            elemRepos;
    FsaElemReposCnstr       automReposCnstr(  &rexpRepos, &elemRepos); 
    automReposCnstr.Process(); 

    //elemRepos.Dump( std::cout);
    if ( m_ElemDotFile.size())
    {
        std::ofstream       fsaOStrm( m_ElemDotFile);
        Cv_DotStream	    fsaDotStrm( &fsaOStrm, true);  
        elemRepos.WriteDot( fsaDotStrm);
    }  
    elemRepos.DumpStats( std::cout);
    FsaDfaRepos             dfaRepos;
    FsaDfaCnstr             dfaCnstr( &elemRepos, &dfaRepos);

    if ( !m_DfaDotFile.size() && !m_ImgFile.size() && !m_DataFile.size())
        return apiErrCode;

    dfaCnstr.SubsetConstruction();
    dfaRepos.DumpStats( std::cout);
    dfaRepos.m_DistribRepos.Dump( std::cout);

    if ( m_DfaDotFile.size())
    {
        std::ofstream           fsaOStrm( m_DfaDotFile);
        Cv_DotStream			fsaDotStrm( &fsaOStrm, true);  
        dfaRepos.WriteDot( fsaDotStrm);
    }
    
    if ( m_ImgFile.size())
    {
        {
            Cv_FileSpritz           imgSpritz( m_ImgFile, Cv_FileSpritz::WriteTrim); 
            Cv_ValidationSpritz     valSpritz( &imgSpritz); 

            Cv_Aid::Save( &valSpritz, dfaRepos);
            //Cv_Aid::Save( &valSpritz, &dfaRepos.m_DistribRepos);
            bool t = true;
        }        
        if ( m_ImgDotFile.size()) 
        {
            std::vector< uint8_t>   memArr;
            bool	                res = Cv_Aid::ReadVec( &memArr, m_ImgFile.c_str()); 
            FsaDfaRepos::Blossom    blossom(  &memArr[ 0]);  
            std::ofstream           fsaOStrm( m_ImgDotFile.c_str());
            Cv_DotStream			fsaDotStrm( &fsaOStrm, true);  
            auto                    states = blossom.States();
            FsaDfaRepos::Id         rootId = blossom.RootId();
            for ( uint32_t i = 0; i < states.Size(); ++i)
            {
                auto        var = states.VarAt( i); 
                bool t = true;
                if (var)
                    var( [ i, var, &fsaDotStrm]( auto k) { k->DumpDot( Cv_CrateId( i, var.GetType()), fsaDotStrm); });
            }
            bool t = true;
           
            DistribRepos::Blossom   distribs = blossom.Distribs();
            auto                    dVar = distribs.ToVar( DistribRepos::Id( 0, 5));  
            uint8_t                 chrId = 25;
            uint8_t                 img = dVar( [ chrId]( auto k) { return k->Image( chrId); }); 
            bool t1 = true;
        }
    }
    if ( m_DataFile.size())
    {
        StrInStream			    dataMemVector;
        bool	                res1 = Cv_Aid::ReadVec( dataMemVector.CharVec(), m_DataFile.c_str()); 
        if ( !res1)
        {
            std::cerr << "Not Found : " << m_DataFile << '\n';
            return -1;
        }
        typedef Cv_Array< Sg_MatchData, 256>            MatchArr;
        Sg_DfaReposAtelier                              atelier( &dfaRepos);  
        MatchArr                                        matches;
        Sg_Citadel< Sg_DfaReposAtelier, MatchArr>  bulwark;  
        bulwark.Setup( &atelier);
        bulwark.m_TokenSet = &matches;

        bulwark.Play( Cv_Seq( ( uint8_t *) ( void *) &dataMemVector.CharVec()->at( 0), ( uint32_t ) dataMemVector.CharVec()->size()));
        for ( uint32_t k = 0; k < matches.SzFill(); ++k)
            std::cout << matches[ k];
        matches = MatchArr();

/*        for ( uint32_t i = 0; i < dataMemVector.CharVec()->size(); ++i)
        {
            uint8_t     chrId = atelier.ByteCode(  ); 
            
        }*/
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
    return apiErrCode;
}

//_____________________________________________________________________________________________________________________________ 

int     Sg_AnealCmdProcessor::Execute( void)
{
    Sg_Partition        prtn;
    Sg_ChSet            ws = Sg_ChSet::Word();
    Sg_ChSet            digit = Sg_ChSet::Digit();
    //prtn.ImpressCCL( ws);
    //prtn.ImpressCCL( digit);
    //std::cout << prtn.ToString() << '\n';

    Sg_Bitset< 7>       a;
    Sg_Bitset< 64>      b;
    Sg_Bitset< 194>     c;

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

