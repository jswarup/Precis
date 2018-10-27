// sg_cmdtimbre.cpp ____________________________________________________________________________________________________________ 


#include    "trellis/tenor/tr_include.h" 
#include    "cove/snip/cv_cmdexec.h" 
#include    "trellis/swarm/tr_worker.h" 
#include    "trellis/stalks/tr_ringbuf.h"
#include    "trellis/stalks/tr_datacarousal.h"

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
    {   }

    int     Execute( void);
    int     Test(void);

    bool    ProcessProgArgs( std::istream &cmdStrm)
    {
        std::string             tok;
        return !( cmdStrm >> tok) && tok.empty();
    }

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

struct Tr_Consume
{
    typedef Tr_DataCarousal<uint64_t>     DataCarousal;
    
    uint64_t                    m_Prev;
    Tr_DataDock< uint64_t>      m_Dock;
    
    Tr_Consume( void)
        : m_Prev( CV_UINT64_MAX)
    {}

    void    InitSetup( DataCarousal *pRingBuf)
    {        
        m_Dock.Setup( pRingBuf);
    }

    void    DoRun( void)
    {  
        while( true)
        {
            uint32_t    sz = m_Dock.Summon(); 
            if ( !sz) 
                continue; 
            uint32_t i = 0;
            for ( ; i < sz;)
            {
                uint32_t    val = m_Dock.Get( i++);
                printf( "%llu\n", val);
                CV_ERROR_ASSERT( ( m_Prev == CV_UINT64_MAX)  || ( m_Prev < val))
                m_Prev = val;
                if ( val == (cv_numEventsToGenerate -1))
                {
                    m_Dock.Commit( i);
                    return;
                } 
            }
            m_Dock.Commit( i);
        } 
        return;
    }
};

//_____________________________________________________________________________________________________________________________

struct Tr_Produce 
{
    typedef Tr_DataCarousal< uint64_t>   DataCarousal;
    Tr_DataDock< uint64_t>              m_Dock;
    

    Tr_Produce( void) 
    {}

    void    InitSetup( DataCarousal *pRingBuf)
    {        
        m_Dock.Setup( pRingBuf);
    }

    void    DoRun( void)
    { 
         
        for ( uint64_t num_event = 0; num_event < cv_numEventsToGenerate; )
        { 
            uint32_t    sz = m_Dock.Summon();
            if ( !sz)
                continue;
            uint32_t i = 0;
            for ( ; ( i < sz) && ( num_event < cv_numEventsToGenerate); ++i)
                m_Dock.Set( i, num_event++);
            m_Dock.Commit( i);
        } 
        return;
    }
}; 

//_____________________________________________________________________________________________________________________________

static int TestProduce( void)
{
    
    typedef Tr_DataCarousal< uint64_t>     DataCarousal;

    DataCarousal            ringBuf; 
    Tr_Produce              producer;
    producer.InitSetup( &ringBuf);

    Tr_Consume              consumer; 
    consumer.InitSetup( &ringBuf);   

    Tr_Consume              consumer1; 
    consumer1.InitSetup( &ringBuf); 
  
    Tr_Consume              consumer2; 
    consumer2.InitSetup( &ringBuf);    
 
    std::thread             t2{ &Tr_Consume::DoRun, &consumer2};
    std::thread             t{ &Tr_Consume::DoRun, &consumer};
    std::thread             t1{ &Tr_Consume::DoRun, &consumer1};

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

