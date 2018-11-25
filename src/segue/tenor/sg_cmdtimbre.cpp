// sg_cmdtimbre.cpp ____________________________________________________________________________________________________________ 


#include    "segue/tenor/sg_include.h" 
#include    "cove/snip/cv_cmdexec.h"
#include    "segue/timbre/sg_timbreparser.h"

#include <utility>
#include  <tuple>

///_____________________________________________________________________________________________________________________________ 

static Cv_CmdOption     s_TimbreIfcOptions[] = 
{
     
    { 0, 0,  0},
};

//_____________________________________________________________________________________________________________________________ 

class Sg_TimbreCmdProcessor : public Cv_CmdExecutor
{
     

public:
    Sg_TimbreCmdProcessor( void)  
    {

    }

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
 
CV_CMD_DEFINE( Sg_TimbreCmdProcessor, "timbre", "timbre", s_TimbreIfcOptions)

//_____________________________________________________________________________________________________________________________ 

int     Sg_TimbreCmdProcessor::Test(void)
{
    auto        add_lambda = [](auto first, auto second) { return first + second; };
    std::cout << std::apply( add_lambda, std::make_pair(2.0f, 3.0f)) << '\n'; 
    return 0;
}

//_____________________________________________________________________________________________________________________________ 

int     Sg_TimbreCmdProcessor::Execute( void)
{
    //AC_API_BEGIN() 
    Test();
    bool    apiErrCode = false;
    
    {
        using namespace Sg_Timbre;
        
        InStream    inStream( "alltelltest");
        Parser      parser( &inStream);
        auto        lRgx = Str( "l") [( []( auto ctxt) {  
            std::cout << ctxt.MatchStr() << "\n"; 
            return true; 
        }) ];  
        auto        regex = Str( "a") >> (( *lRgx | IStr("te")) >> Str( "test"))[ ( []( auto ctxt) { 
            std::cout << ctxt.MatchStr() << "\n";  
            return true; 
        }) ];   
            

        apiErrCode = StrParser( "alltest").Match( &regex);
        
        Cv_CrateRepos< SynParserCrate>               synCrate ;
        Cv_CrateConstructor< SynParserCrate>		synCnstr( &synCrate);
        auto                    synElem = synCnstr.FetchElem( &regex);
        std::ofstream           ostrm( "a.dot");
        Cv_DotStream            synDotStrm( &ostrm, false); 
        synCrate.OperateAll(  [&synDotStrm]( auto k ){
            k->WriteDot( synDotStrm);
            return true;
         }); 
        synElem->Operate<SynParserCrate>( [&synDotStrm]( auto k ){
            k->WriteDot( synDotStrm);
            return true;
         });
        synCrate.Clear();
        bool                    t = true; 
    };
        
    //AC_API_END()
    return apiErrCode;
}

//_____________________________________________________________________________________________________________________________ 

