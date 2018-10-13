//  cv_dotstream.h ___________________________________________________________________________________________________________________
#pragma once 
 
//_____________________________________________________________________________________________________________________________ 

class  Cv_DotStream
{ 
    std::ostream            *m_OStrm;

public:
    Cv_DotStream( std::ostream *strm, bool vertFlg=true)
        :   m_OStrm( strm)
    {
        *m_OStrm << "digraph graphname { \n";
        if ( !vertFlg)
            *m_OStrm << " rankdir=LR;";
        *m_OStrm << "concentrate=false;\n";
    }

    ~Cv_DotStream( void)
    {
        *m_OStrm <<  "}\n";
    }

    std::ostream    &OStream( void) { return *m_OStrm; } 
  
    Cv_DotStream    &operator<<( const Cv_ReposEntry *x)  
    { 
        *m_OStrm << x->GetName() << '_' <<  x->GetId();
        return SELF;
    } 
    
    Cv_DotStream   &operator<<( const char *x)
    {
        for ( ; *x; x++)
            SELF << *x;
        return SELF;
    } 
    
    Cv_DotStream   &operator<<( const std::string &y)
    {
        SELF << y.c_str();
        return SELF;
    }

    Cv_DotStream   &operator<<( const char &x)
    {
        switch ( x)
        {
            case '\\':  *m_OStrm <<  "\\\\"; break;
            default :  *m_OStrm <<  x; break;
        }
        return SELF;
    }

    void    HtmlDump( const std::string &y)
    {
        for ( uint32_t ci = 0; ci < y.length(); ++ci)
        {
            char    x = y[ ci];
            switch ( x)
            {
                case '\n':  *m_OStrm <<  " <BR/> "; break;
                default :  *m_OStrm <<  x; break;
            }
        }
    }
        
    static const char *Color( uint32_t k) 
    {
        static const char   *color[] = {
            "red",
            "blue",
            "green",
            "cyan",
        };

        static uint32_t     ncolor = sizeof( color)/ sizeof( char *);
        return color[ k % ncolor];
    }
};


//-------------------------------------------------------------------------------------------------
 