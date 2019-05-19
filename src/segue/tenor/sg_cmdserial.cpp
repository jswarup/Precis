// sg_cmdtimbre.cpp ____________________________________________________________________________________________________________ 


#include    "segue/tenor/sg_include.h" 
#include    "cove/snip/cv_cmdexec.h"
#include    "segue/timbre/sg_timbreparser.h"
#include    "segue/timbre/sg_partition.h"

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
    int                         m;
    std::vector< uint32_t>      vec; 

    struct Cask : public Cv_MemberCask< std::vector< uint32_t >, int>
    {
        typedef Cv_MemberCask< std::vector< uint32_t>, int>    BaseCask; 

        bool        Serialize( const Test23 &t, Cv_Spritz *spritz) { return BaseCask::Serialize( t.vec, t.m, spritz); }
    };
}; 

//_____________________________________________________________________________________________________________________________ 

int     Sg_SerializeCmdProcessor::Test(void)
{
    {
        Cv_FileSpritz           imgSpritz( "a.txt", Cv_FileSpritz::WriteTrim);
    
        std::vector< uint32_t>  vec;
        vec.push_back( 80);
        vec.push_back( 67);
        vec.push_back( 32);
        bool                   t = true;
     
        Cv_CArr< uint32_t>     arr( &vec.at( 0), uint32_t( vec.size()));
        int                     t1 = 0;
        Test23                  t23;
        t23.m = 137;
        t23.vec = vec;

        Sg_CharPartition< 64>       charPrtn;    
        Sg_Bitset< 64>              ccl;
        ccl.SetByteRange( 5, 13, true);
        charPrtn.ImpressCCL( ccl);
    
        Sg_Bitset< 64>              ccl1;
        ccl1.SetByteRange( 45, 53, true);
        charPrtn.ImpressCCL( ccl1);
    
        Cv_Aid::Save( charPrtn, &imgSpritz);
        Cv_Aid::Save( t23, &imgSpritz);
        //Cv_Cask< int *>   ser( &t1);
        //ser.Serialize( &imgSpritz);
    }
    {
        Cv_FileSpritz       imgSpritz( "a.txt", Cv_FileSpritz::ReadOnly);
        auto                ct = Cv_Cask< Sg_CharPartition< 64>>().Bloom(  &imgSpritz);
        ct.Dump( std::cout);
       // auto                ct1 = Cv_Cask< Test23>().Bloom( &imgSpritz);
        std::cout << '\n';
        //std::cout << ct1.m_Size << ' ' << ct1.m_Size << '\n';
        
        std::cout << '\n';
    }

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

