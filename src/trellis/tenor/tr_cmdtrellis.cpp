// sg_cmdtimbre.cpp ____________________________________________________________________________________________________________ 


#include    "trellis/tenor/tr_include.h" 
#include    "cove/snip/cv_cmdexec.h" 
#include    "trellis/swarm/tr_worker.h" 

#include    <utility>
#include    <tuple>

///_____________________________________________________________________________________________________________________________ 

static Cv_CmdOption     s_TrellisIfcOptions[] = 
{
     
    { 0, 0,  0},
};

//_____________________________________________________________________________________________________________________________ 

class Sg_TrellisCmdProcessor : public Cv_CmdExecutor
{
     

public:
    Sg_TrellisCmdProcessor( void)  
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
 
CV_CMD_DEFINE( Sg_TrellisCmdProcessor, "trellis", "trellis", s_TrellisIfcOptions)

//_____________________________________________________________________________________________________________________________ 

int     Sg_TrellisCmdProcessor::Test(void)
{
    auto        add_lambda = [](auto first, auto second) { return first + second; };
    std::cout << std::apply( add_lambda, std::make_pair(2.0f, 3.0f)) << '\n'; 
    return 0;
}

//_____________________________________________________________________________________________________________________________ 

int     Sg_TrellisCmdProcessor::Execute( void)
{
    //AC_API_BEGIN() 
    Test();
    bool                apiErrCode = false;
    Tr_WorkShop         workShop;        
        
    apiErrCode = workShop.DoRun();
    //AC_API_END()
    return apiErrCode;
}

//_____________________________________________________________________________________________________________________________ 

