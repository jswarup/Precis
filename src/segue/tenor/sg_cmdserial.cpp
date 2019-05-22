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

    struct Cask : public Cv_MemberCask< std::vector< uint32_t>, int>
    {  
        typedef Cv_MemberCask< std::vector< uint32_t>, int>     BaseCask; 
        typedef typename BaseCask::ContentType                  BaseContent;

        struct  ContentType : public BaseContent
        {  
            ContentType(  const BaseContent &t2)
                : BaseContent( t2)
            {}
            
            uint64_t    GetSize( void) { return m_Value.m_Size; } 
            auto        GetVec(  uint8_t *arr) { return m_Value.Value( arr); }
            auto        GetM( void) { return ((BaseCask::BaseContent *) this)->m_Value; }
        };

        ContentType     Encase( Cv_Spritz *spritz, const Test23 &obj)
        { 
            return BaseCask::Encase( spritz, obj.vec, obj.m);
        }
 
        ContentType     *Bloom( uint8_t *arr)
        {
            return ( ContentType *) arr;
        }
    }; 

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
        imgSpritz.SetOffsetAtEnd();

        std::vector< uint32_t>  vec;
        vec.push_back( 80);
        vec.push_back( 67);
        vec.push_back( 32); 
 
        

        Test23                  t23;
        t23.m = 137;
        t23.vec = vec;
        Cv_Aid::Save( &imgSpritz, t23);
        imgSpritz.SetOffsetAtEnd();

        Cv_Aid::Save( &imgSpritz, vec);
        imgSpritz.SetOffsetAtEnd();

    }
    {
        std::vector< char>  charVec;
        bool	            res = Cv_Aid::ReadVec( &charVec, "a.txt"); 
        Cv_CArr< uint8_t>   memArr( ( uint8_t *) &charVec.at( 0), uint32_t( charVec.size()));
 
        auto                ct = Cv_Cask< Sg_CharPartition< 64>>().Bloom( memArr.Ptr());
        ct->Dump( std::cout);
        memArr = memArr.Ahead( Cv_Cask< Sg_CharPartition< 64>>().Spread( ct, memArr.Ptr()));
 
        auto                ct2 = Test23::Cask().Bloom( memArr.Ptr());
        bool    t1 = true;
        std::cout << ct2->GetM() << ' ' << ct2->GetSize() <<  '\n';
        auto                arr1 = ct2->GetVec( memArr.Ptr());
        for ( uint32_t i = 0; i < arr1.Size(); ++i)
            std::cout << arr1[ i] << ' ';
        std::cout  <<  '\n';
        memArr = memArr.Ahead( Cv_Cask<Test23>().Spread( ct2, memArr.Ptr()));

        auto        ct1 = Cv_Cask<  Cv_CArr< uint32_t>>().Bloom( memArr.Ptr());
        auto        arr = ct1->Value( memArr.Ptr());
        for ( uint32_t i = 0; i < arr.Size(); ++i)
            std::cout << arr[ i] << ' ';
        std::cout << '\n';
        memArr = memArr.Ahead( Cv_Cask<Cv_CArr< uint32_t>>().Spread( ct1, memArr.Ptr()));
        bool t = true;
     
        
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

