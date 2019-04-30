// cv_cstr.h ______________________________________________________________________________________________________________
#pragma once


#include 	<string.h>

//_____________________________________________________________________________________________________________________________

class Cv_CStr 
{
protected:
    const char      *m_CStr;
    uint32_t        m_Len;

public:
template < std::size_t N>
    Cv_CStr( const char( &s)[N]) 
        : m_CStr( s), m_Len (N - 1)
    {} 

	Cv_CStr( const char *ptr = nullptr) 
        : m_CStr( ptr), m_Len( ptr ? uint32_t( strlen( ptr)) : 0)
    {}

    Cv_CStr( const char *ptr, uint32_t len) 
        : m_CStr( ptr), m_Len( len)
    {}

    Cv_CStr( const char *ptr, const char *end) 
        : m_CStr( ptr), m_Len( uint32_t( end -ptr))
    {}

    Cv_CStr( const std::string &str) 
        : m_CStr( str.c_str()), m_Len( uint32_t( str.length()))
    {}

    //_____________________________________________________________________________________________________________________________

    const char              *CStr( void) const { return m_CStr; }   

    operator                std::string( void) const { return std::string( m_CStr, m_CStr+m_Len); }

    uint32_t                Size( void) const { return m_Len; }

    const char              &operator[]( uint32_t i) const { return m_CStr[ i]; }

    int32_t                 Compare( const Cv_CStr &rhs) const 
    {
        int32_t     res = strncmp( m_CStr, rhs.m_CStr, Cv_CExpr::Min( Size(), rhs.Size()));
        return res ? res : ( Size() == rhs.Size() ? 0 : ( Size() < rhs.Size() ? -1 : 1)) ;
    }

    //_____________________________________________________________________________________________________________________________

    friend bool             operator==( const Cv_CStr &lhs, const Cv_CStr &rhs) { return ( lhs.Size() == rhs.Size()) && ( strncmp( lhs.m_CStr, rhs.m_CStr, rhs.Size()) == 0); }
    friend bool             operator==( const Cv_CStr &lhs, const std::string &rhs) { return ( lhs.Size() == rhs.length()) && ( strncmp( lhs.m_CStr, rhs.c_str(), rhs.length()) == 0); }
    friend bool             operator==( const std::string &lhs, const Cv_CStr &rhs) { return rhs == lhs; }
    friend bool             operator==( const Cv_CStr &lhs, const char *rhs) { return lhs == Cv_CStr( rhs); }
    friend bool             operator==( const char *lhs, const Cv_CStr &rhs) { return rhs == lhs; }
                                        
    friend bool             operator!=( const Cv_CStr &lhs, const Cv_CStr &rhs) { return !( lhs == rhs); }
    friend bool             operator!=( const Cv_CStr &lhs, const std::string &rhs) { return !(lhs == rhs); }
    friend bool             operator!=( const std::string &lhs, const Cv_CStr &rhs) { return !(lhs == rhs); }
    friend bool             operator!=( const Cv_CStr &lhs, const char *rhs) { return !(lhs == rhs); }
    friend bool             operator!=( const char *lhs, const Cv_CStr &rhs) { return !(lhs == rhs); }

    friend bool             operator>( const Cv_CStr &lhs, const Cv_CStr &rhs) { return rhs < lhs; }
    friend bool             operator<( const Cv_CStr &lhs, const Cv_CStr &rhs) { return lhs.Compare( rhs) < 0; }
    
    //_____________________________________________________________________________________________________________________________

    friend std::ostream     &operator<<( std::ostream  &ostr, const Cv_CStr &cstr)
    {
        ostr.write( cstr.CStr(), cstr.Size());
        return ostr;
    }

    //_____________________________________________________________________________________________________________________________

    Cv_CStr     &Advance( uint32_t k)
    {
        if ( k > m_Len)
            k = m_Len;
        m_Len -= k;
        m_CStr = m_Len ? ( m_CStr + k) : nullptr;
        return SELF;
    }

    //_____________________________________________________________________________________________________________________________

    Cv_CStr     &Shorten( uint32_t k)
    {
        CV_ERROR_ASSERT( m_Len)
        m_Len -= k;
        if ( !m_Len)
            m_CStr = nullptr;
        return SELF;
    }

    //_____________________________________________________________________________________________________________________________

    Cv_CStr     &Stretch( uint32_t k)
    {
        m_Len += k;
        return SELF;
    }

    //_____________________________________________________________________________________________________________________________

    Cv_CStr    TrimFrontWhite( bool nlwsFlg) const
    {
        const char  *ch = m_CStr;
        const char  *end = m_CStr + m_Len;
        for ( ; ch < end; ++ch)
            if (( *ch != ' ') && ( *ch != '\t') &&  ( *ch != '\r') && ( !nlwsFlg || (( *ch != '\n') && ( *ch != '\r'))))
                break;
        return Cv_CStr( ch, m_Len -uint32_t( ch -m_CStr));
    }

    //_____________________________________________________________________________________________________________________________

    Cv_CStr     TrimBackWhite( bool nlwsFlg) const
    {
        const char  *ch = m_CStr + m_Len;
        for ( --ch; m_CStr <= ch; --ch)
            if (( *ch != ' ') && ( *ch != '\t') &&  ( *ch != '\r') && ( !nlwsFlg || (( *ch != '\n') && ( *ch != '\r'))))
                break;
        return Cv_CStr( m_CStr, ++ch);
    }
    
    //_____________________________________________________________________________________________________________________________

    uint32_t     Locate( char c) const
    {
        for ( uint32_t i = 0; i < m_Len; ++i)
            if ( m_CStr[ i] == c)
                return i;
        return CV_UINT32_MAX;
    }

    //_____________________________________________________________________________________________________________________________

    uint32_t     RLocate( char c) const
    {
        for ( uint32_t i = 0; i < m_Len; ++i)
            if ( m_CStr[ m_Len -1 -i] == c)
                return m_Len -1 -i;
        return CV_UINT32_MAX;
    }

    //_____________________________________________________________________________________________________________________________

    uint32_t    ToUnsignedDec( void)
    {
        uint32_t    num = 0;
        for ( const char *ch = m_CStr, *end = m_CStr + m_Len; ch < end; ++ch)
        {
            if (( *ch < '0') || ( *ch > '9'))
                return CV_UINT32_MAX;
            num = num * 10 + uint32_t( *ch - '0');
        }
        return num;

    }
        
    //_____________________________________________________________________________________________________________________________
};

//_____________________________________________________________________________________________________________________________

class Cv_OStr
{
    Cv_CStr     m_CStr;
    bool        m_Owned;

public:
    Cv_OStr( const Cv_CStr &cstr, bool owned)
        : m_CStr( cstr), m_Owned( owned)
    {
        if ( m_Owned)
        {
            char    *str = new char[ m_CStr.Size()];
            memcpy( str, m_CStr.CStr(), m_CStr.Size());  
            m_CStr = Cv_CStr( str, m_CStr.Size());
        }
    }
    ~Cv_OStr( void)
    {   
        if ( m_Owned)
            delete [] cv_pcast< char>( m_CStr.CStr());
    }

    const Cv_CStr   &CStr( void) const { return m_CStr; }
};

//_____________________________________________________________________________________________________________________________


