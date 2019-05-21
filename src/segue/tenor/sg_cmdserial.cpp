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
 /*
    struct Cask : public Cv_MemberCask< std::vector< uint32_t>, int>
    {  
        typedef Cv_MemberCask< std::vector< uint32_t>, int>     BaseCask; 
        typedef typename BaseCask::ContentType                  BaseContent;

        struct  ContentType : public BaseContent
        {  
            ContentType(  const BaseContent &t2)
                : BaseContent( t2)
            {}
            
            uint64_t         GetSize( void) { return m_Value.m_Size; } 
            auto             GetM( void) { return ((BaseCask::BaseContent *) this)->m_Value; }
        };

        bool        Serialize( Cv_Spritz *spritz, const Test23 &t) { return BaseCask::Serialize( spritz, t.vec, t.m); }

        ContentType     Bloom( Cv_Spritz *spritz)
        {
            ContentType     obj( BaseCask::Bloom( spritz)); 
            return obj;
        }
    };
 */  

}; 

//_____________________________________________________________________________________________________________________________ 

int     Sg_SerializeCmdProcessor::Test(void)
{
    {
        Cv_FileSpritz           imgSpritz( "a.txt", Cv_FileSpritz::WriteTrim);
        Sg_CharPartition< 64>       charPrtn;    
        Sg_Bitset< 64>              ccl;
        ccl.SetByteRange( 5, 13, true);
        charPrtn.ImpressCCL( ccl);

        Sg_Bitset< 64>              ccl1;
        ccl1.SetByteRange( 45, 53, true);
        charPrtn.ImpressCCL( ccl1);

        Cv_Aid::Save( &imgSpritz, charPrtn);

        std::vector< uint32_t>  vec;
        vec.push_back( 80);
        vec.push_back( 67);
        vec.push_back( 32); 

        Cv_CArr< uint32_t>     arr( &vec.at( 0), uint32_t( vec.size()));
        Cv_Aid::Save( &imgSpritz, arr);
    }
    {
        std::vector< char>  charVec;
        bool	            res = Cv_Aid::ReadVec( &charVec, "a.txt"); 
        Cv_CArr< uint8_t>   memArr( ( uint8_t *) &charVec.at( 0), uint32_t( charVec.size()));
        auto                ct = Cv_Cask< Sg_CharPartition< 64>>().Bloom( memArr);
        ct->Dump( std::cout);
        memArr = memArr.Ahead( sizeof( *ct));
        auto        ct1 = Cv_Cask<  Cv_CArr< uint32_t>>().Bloom( memArr);
        auto        arr = ct1->Value( memArr);
        bool t = true;
    }   
   /* {
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
        Cv_Aid::Save( &imgSpritz, t23);
/*
        Sg_CharPartition< 64>       charPrtn;    
        Sg_Bitset< 64>              ccl;
        ccl.SetByteRange( 5, 13, true);
        charPrtn.ImpressCCL( ccl);
    
        Sg_Bitset< 64>              ccl1;
        ccl1.SetByteRange( 45, 53, true);
        charPrtn.ImpressCCL( ccl1);
    
        Cv_Aid::Save( &imgSpritz, charPrtn);
*/
//        Cv_Aid::Save( &imgSpritz, t23);
        //Cv_Cask< int *>   ser( &t1);
        //ser.Serialize( &imgSpritz);
/*    }
    if ( 1)
    {
        Cv_FileSpritz       imgSpritz( "a.txt", Cv_FileSpritz::ReadOnly);
      //  auto                ct = Cv_Cask< Sg_CharPartition< 64>>().Bloom(  &imgSpritz);
     //   ct.Dump( std::cout);
      //  auto                ct1 = Cv_Cask< Test23>().Bloom( &imgSpritz);
      //  std::cout << '\n';
      //  std::cout << ct1.GetM() <<  ' ' << ct1.GetSize() << '\n';
        
        std::cout << '\n';
    }
    {
        std::vector< char>  charVec;
        bool	                res = Cv_Aid::ReadVec( &charVec, "a.txt"); 
        Cv_CArr< uint8_t>   memArr( ( uint8_t *) &charVec.at( 0), charVec.size());
     //   auto                ct = Cv_Cask< Sg_CharPartition< 64>>().Blossom( &memArr);
     //   ct->Dump( std::cout);
       // memArr.Advance( Cv_Cask< Sg_CharPartition< 64>>().ObjLen());
        auto                ct1 = Cv_Cask< Test23>().Blossom( &memArr);
        bool    t = true;
    }
*/
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

