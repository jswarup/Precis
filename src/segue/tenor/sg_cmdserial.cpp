// sg_cmdtimbre.cpp ____________________________________________________________________________________________________________ 


#include    "segue/tenor/sg_include.h" 
#include    "cove/snip/cv_cmdexec.h"
#include    "segue/timbre/sg_timbreparser.h"

#include <utility>
#include  <tuple>

///_____________________________________________________________________________________________________________________________ 

static Cv_CmdOption     s_SerializeIfcOptions[] = 
{
     
    { 0, 0,  0},
};

//_____________________________________________________________________________________________________________________________ 

class Sg_SerializeCmdProcessor : public Cv_CmdExecutor
{
     

public:
    Sg_SerializeCmdProcessor( void)  
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
 
CV_CMD_DEFINE( Sg_SerializeCmdProcessor, "serial", "serial", s_SerializeIfcOptions)


struct  Test23
{
    int         *m;
};

//_____________________________________________________________________________________________________________________________ 

int     Sg_SerializeCmdProcessor::Test(void)
{
    Cv_FileSpritz           imgSpritz( "a.txt", Cv_FileSpritz::WriteTrim);
    
    std::vector< uint32_t>  vec;
    vec.push_back( 80);
    vec.push_back( 67);
    vec.push_back( 32);
    bool                   t = true;
    Cv_Serializer< std::vector< uint32_t> > serializer( &vec);
    serializer.Save( &imgSpritz);
    //Cv_Aid::Save( &vec, &imgSpritz);
    return 0;
}

//_____________________________________________________________________________________________________________________________ 

int     Sg_SerializeCmdProcessor::Execute( void)
{
    //AC_API_BEGIN() 
    Test(); 
    //AC_API_END()
    return 0;
}

//_____________________________________________________________________________________________________________________________ 

