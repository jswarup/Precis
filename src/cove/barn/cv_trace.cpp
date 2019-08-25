// cv_trace.cpp _______________________________________________________________________________________________________________


#include    "cove/barn/cv_include.h"
#include    "cove/barn/cv_trace.h"
#include    <stdarg.h>
 
#ifdef CV_TRACEDBG

//-----------------------------------------------------------------------------------------------------------------------------

Cv_Logger   Cv_Logger::s_Instance;   

//-----------------------------------------------------------------------------------------------------------------------------

Cv_Logger::Cv_Logger( void)
{
    
}

//-----------------------------------------------------------------------------------------------------------------------------

void   Cv_Logger::LogBook::Indent( void)
{
    fprintf( m_File, "\n");
    for  ( uint32_t i = 1; i < m_Level; ++i) 
        fprintf (m_File, "    ");
    m_Innermost = false;
    return;
} 

//-------------------------------------------------------------------------------------------------

void    Cv_Logger::LogBook::RaiseLevel( const char *tpName, int count)
{
    ++m_Level; 
    Indent();
    fprintf( m_File, "%s #%d {", tpName, count);
    m_Innermost = true;
}

//-------------------------------------------------------------------------------------------------

void    Cv_Logger::LogBook::LowerLevel( void)
{
    if ( !m_Innermost)
        Indent();
    
    fprintf( m_File, "}"); 
    fflush ( m_File);    
    --m_Level;
    m_Innermost = false;
    return;
}
   
//-----------------------------------------------------------------------------------------------------------------------------

void    Cv_Logger::LogBook::PrintTrace (const char *fmt, va_list arg)
{
    if ( !m_File)
        return;
    vfprintf( m_File, fmt, arg);
    return;
}

//-----------------------------------------------------------------------------------------------------------------------------

void    Cv_Logger::LogBook::Print (const char *fmt, ...)
{ 
    va_list        arg;
    va_start (arg, fmt);   
    PrintTrace (fmt, arg);
    va_end (arg );  
    return; 
}

//-----------------------------------------------------------------------------------------------------------------------------

void    Cv_Logger::LogEntry::Trace( const char *fmt, ...)
{
    va_list             arg;
    va_start (arg, fmt);   
    Book()->PrintTrace (fmt, arg);
    va_end (arg );     
    return;
}

//-----------------------------------------------------------------------------------------------------------------------------

bool    Cv_Logger::LogPage::MarkEntry( LogEntry *entry)
{
    m_Counts[ entry->ThreadId()] += 1;  
    LogBook      *logBook = entry->Book();;
    logBook->RaiseLevel( m_Name, m_Counts[ entry->ThreadId()]); 
    logBook->Push( entry);
    return true;
}

//-----------------------------------------------------------------------------------------------------------------------------
               
void    Cv_Logger::LogPage::MarkExit( LogEntry *entry)
{
    LogBook     *logBook = entry->Book();
    LogEntry    *topEntry = logBook->Pop();
    CV_SANITY_ASSERT( entry == topEntry) 
    logBook->LowerLevel();
}

//-----------------------------------------------------------------------------------------------------------------------------
#endif

