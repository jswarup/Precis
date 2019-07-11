// sg_cmdtimbre.cpp ____________________________________________________________________________________________________________ 


#include    "trellis/tenor/tr_include.h" 
#include    "cove/barn/cv_cmdexec.h" 
#include    "trellis/swarm/tr_worker.h"  
#include    "cove/stalks/cv_datacarousal.h"

#include    <utility>
#include    <tuple>

///_____________________________________________________________________________________________________________________________ 

static Cv_CmdOption     s_TrellisIfcOptions[] = 
{
    { "-i", "<input>", 0},
    { "-o", "<output>", 0}, 
    { 0, 0,  0}
};

//_____________________________________________________________________________________________________________________________ 

class Sg_TrellisCmdProcessor : public Cv_CmdExecutor
{ 
    std::string     m_InputFile;
    std::string     m_OutputFile;

public:
    Sg_TrellisCmdProcessor( void)  
    {}

    int     Execute( void);
    int     Test(void); 

    bool    ParseArg( const std::string &key, const std::string &arg)
    {
        if ("-i" == key)
        {
            m_InputFile = arg;
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
 
CV_CMD_DEFINE( Sg_TrellisCmdProcessor, "trellis", "trellis", s_TrellisIfcOptions)

static const int cv_numEventsToGenerate = 10000;
  

//_____________________________________________________________________________________________________________________________

struct Tr_ProduceNums
{
    typedef Cv_DataCreek< uint64_t, 32>           Dock;
    
    Dock        m_Dock; 

    Tr_ProduceNums( void) 
    {}

    
    void    DoRun( void)
    { 
         
        for ( uint64_t num_event = 0; num_event < cv_numEventsToGenerate; )
        { 
            Dock::Wharf     wharf( &m_Dock);
            uint32_t        i = 0; 
            for ( ; ( i < wharf.Size()) && ( num_event < cv_numEventsToGenerate); ++i)
                wharf.Set( i, num_event++);
            wharf.SetSize( i);
        } 
        return;
    }
}; 


//_____________________________________________________________________________________________________________________________

struct Tr_ConsumeNums
{
    typedef Cv_DataDock<uint64_t, 32>           Dock;
    typedef typename Dock::DataCarousal     DataCarousal;

    Dock        m_Dock;
    uint64_t    m_Prev;

    Tr_ConsumeNums( void)
        : m_Prev( CV_UINT64_MAX)
    {}

    void    Hookup( Tr_ProduceNums *produce)
    {        
        m_Dock.Connect( produce->m_Dock.Carousal());
    }

    void    DoRun( void)
    {  
        while( true)
        {
            Dock::Wharf     wharf( &m_Dock);
            for ( uint32_t i = 0; i < wharf.Size();  i++)
            {
                uint64_t    val = wharf.Get( i);
                printf( "%u\n", uint32_t( val));
                CV_ERROR_ASSERT( ( m_Prev == CV_UINT64_MAX)  || ( m_Prev < val))
                    m_Prev = val;
                if ( val == (cv_numEventsToGenerate -1))
                {
                    wharf.SetSize( i +1);
                    return;
                } 
            }
        } 
        return;
    }
};
//_____________________________________________________________________________________________________________________________

static int TestProduce( void)
{ 
 
    Tr_ProduceNums              producer;  

    Tr_ConsumeNums              consumer; 
    consumer.Hookup( &producer);   

    Tr_ConsumeNums              consumer1; 
    consumer1.Hookup( &producer); 
  
    Tr_ConsumeNums              consumer2; 
    consumer2.Hookup( &producer);    
 
    std::thread             t2{ &Tr_ConsumeNums::DoRun, &consumer2};
    std::thread             t{ &Tr_ConsumeNums::DoRun, &consumer};
    std::thread             t1{ &Tr_ConsumeNums::DoRun, &consumer1};

    producer.DoRun();
    t1.join();
    t.join();
    t2.join();
    return 0;
}


//_____________________________________________________________________________________________________________________________ 

int     Sg_TrellisCmdProcessor::Test(void)
{
    TestProduce();
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

