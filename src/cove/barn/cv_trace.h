// cv_trace.h  ________________________________________________________________________________________________________________
#pragma once

//-----------------------------------------------------------------------------------------------------------------------------
  
#include    "cove/silo/cv_stack.h"

#ifdef CV_TRACEDBG

#include    <mutex>

//-----------------------------------------------------------------------------------------------------------------------------

class Cv_Logger 
{
public:
    enum {
        MaxThreads = 40,
        SzThreads = MaxThreads +1,
    };
    
    class  LogEntry; 
    
    class LogBook : public Cv_Stack< LogEntry>
    {
        std::string     m_Name;
        FILE            *m_File;
        uint32_t        m_Level;
        bool            m_Innermost;
        
    public:
        LogBook( void)
            : m_File( NULL), m_Level( 0), m_Innermost( false)
        {}
        
        const std::string       &Name( void) const { return m_Name; }

        void    DoInit( const std::string &name)
        {
            m_File = fopen( ( name  + ".trc").c_str(), "w+b");
            m_Name = name;
        }

        void    DoFinish( void)
        {
            fclose( m_File);
        }

        void    Indent( void); 
        void    RaiseLevel( const char *tpName, int count);
        void    LowerLevel( void);
        void    PrintTrace( const char *fmt, va_list arg);
        void    Print( const char *fmt, ...);  
        void    Write( const char *buf, uint32_t len) { fwrite(  buf, 1, len, m_File); }
        
    };
    
    class  LogPage 
    {
    protected:
        const char      *m_Name;
        uint32_t        m_Counts[ SzThreads];

    public:

        //-------------------------------------------------------------------------------------------------
        // Each tracepoint belongs to a Sentry, has a name and tracelevel.
        LogPage( const char *nm) 
            :   m_Name( nm)
        {
            memset( &m_Counts[ 0], 0, sizeof( m_Counts));
        }
        const char  *GetName( void) const { return m_Name; }
        uint32_t    GetCount( uint32_t thId = MaxThreads) const { return m_Counts[ thId]; }
  
        bool        MarkEntry( LogEntry *entry);                   
        void        MarkExit( LogEntry *entry);                    
    };
    
    class  LogEntry : public Cv_StackVar< LogEntry>
    {
        LogPage             *m_Page;     
        bool                m_ActiveFlg;
        uint32_t            m_ThreadId;
        
    public:
        LogEntry( LogPage *page, uint32_t thId = MaxThreads)
            :  m_Page( page), m_ThreadId( thId)
        {
            m_ActiveFlg = m_Page->MarkEntry( this);
        }
        ~LogEntry( void)
        {
            if ( m_ActiveFlg)
                m_Page->MarkExit( this);
        }   
        uint32_t    ThreadId( void) const { return m_ThreadId; }
        LogBook     *Book( void) const { return FetchBook( m_ThreadId); }
        void        Trace( const char *fmt, ...);
        void        Trace( void) {}
    };

private:
    static Cv_Logger    s_Instance;
    
    std::mutex        m_Mutex;
    LogBook             m_ThreadFiles[ SzThreads]; 
    
    Cv_Logger( void);
public:
    
    static void    DoInit( const std::string &name)
    {
        s_Instance.m_ThreadFiles[ MaxThreads].DoInit( name ); 
    }

    static LogBook      *FetchBook( uint32_t thId = MaxThreads) 
    {         
        LogBook     *logBook = &s_Instance.m_ThreadFiles[ thId];
        if ( thId == MaxThreads) 
            return logBook;
        if ( logBook->Name().empty())
        {
            std::lock_guard< std::mutex>     lock( s_Instance.m_Mutex); 
            if ( logBook->Name().empty()) 
            {
                std::stringstream   sstr;
                sstr << s_Instance.m_ThreadFiles[ MaxThreads].Name() << '_' << thId;
                logBook->DoInit( sstr.str());
            }
        } 
        return logBook; 
    }
    static Cv_Logger   *Singleton( void) { return &s_Instance;   }
};

//-----------------------------------------------------------------------------------------------------------------------------
           
 #define CV_FNTRACE(  Y)                                                                                                        \
    static  Cv_Logger::LogPage    _tracePage( __func__);                                                                    \
    Cv_Logger::LogEntry           _traceEntry(  &_tracePage, Cv_Logger::MaxThreads);                                            \
    _traceEntry.Trace  Y;                                                                                                       \

 #define CV_THFNTRACE( thId, Y)                                                                                                 \
    static  Cv_Logger::LogPage    _tracePage( __func__);                                                                    \
    Cv_Logger::LogEntry           _traceEntry(  &_tracePage, thId);                                                             \
    _traceEntry.Trace  Y;                                                                                                       \


#define CV_LNTRACE( y)                                                                                                            \
    {                                                                                                                           \
        _traceEntry.Book()->Indent();                                                                                           \
        _traceEntry.Trace  y;                                                                                                   \
    }                                                                                           

#else

struct Cv_Logger 
{
    static void    DoInit( const std::string &) {}
};

#define CV_FNTRACE(  Y)   {} 
#define CV_THFNTRACE( thId, Y)  {}
#define CV_LNTRACE( y) {} 

#endif