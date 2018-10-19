// sg_cmdtimbre.cpp ____________________________________________________________________________________________________________ 


#include    "trellis/tenor/tr_include.h" 
#include    "cove/snip/cv_cmdexec.h" 
#include    "trellis/swarm/tr_worker.h" 
#include    "trellis/stalks/tr_ringbuf.h"

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

static const int cv_numEventsToGenerate = 100000000;
 
//_____________________________________________________________________________________________________________________________

struct Tr_Consume
{
    typedef Tr_RingBuffer<uint64_t>     RingBuf;
    
    uint64_t                m_Prev;
    Cv_Reader< RingBuf>     reader;
    
    Tr_Consume( void)
        : m_Prev( CV_UINT64_MAX)
    {}

    void    InitSetup( RingBuf *pRingBuf)
    {        
        reader.SetRing( pRingBuf);
    }

    void    DoRun( void)
    { 
        while( true)
        {
            uint64_t    val = 0; 
            bool        res = reader.Fetch( &val);
            if ( !res) 
                continue; 
            //printf( "%llu\n", val);
            CV_ERROR_ASSERT( ( m_Prev == CV_UINT64_MAX)  || ( m_Prev < val))
            m_Prev = val;
            if ( val == (cv_numEventsToGenerate -1))
                return; 
        }
        return;
    }
};

//_____________________________________________________________________________________________________________________________

struct Tr_Produce 
{
    typedef Tr_RingBuffer<uint64_t>     RingBuf;
    Cv_Writer< RingBuf>                 writer;

    Tr_Produce( void) 
    {}

    void    InitSetup( RingBuf *pRingBuf)
    {        
        writer.SetRing( pRingBuf);
    }

    void    DoRun( void)
    { 
        for ( uint64_t num_event = 0; num_event < cv_numEventsToGenerate; )
        { 
            bool    res  = writer.Put( num_event); 
            if ( !res) 
                continue; 
                    
            ++num_event; 
        }
        writer.Unload();
    }
};

//_____________________________________________________________________________________________________________________________

static int TestConsume( Tr_RingBuffer< uint64_t>    *pRingBuf)
{
    typedef Tr_RingBuffer<uint64_t>     RingBuf;
     
    Tr_Consume              consumer;
    consumer.InitSetup( pRingBuf);
    consumer.DoRun(); 
    return 0;
}

//_____________________________________________________________________________________________________________________________

static int TestProduce( void)
{
    typedef Tr_RingBuffer<uint64_t>     RingBuf;

    RingBuf                 ringBuf; 
    Tr_Produce              producer;
    producer.InitSetup( &ringBuf); 
    std::thread             t{ TestConsume, &ringBuf};
    producer.DoRun();
    t.join();
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

