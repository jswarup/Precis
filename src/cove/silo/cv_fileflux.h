// cv_fileflux.h ____________________________________________________________________________________________________________ 
#pragma once

#include 	"cove/silo/cv_cstr.h"
#include    <fcntl.h> 

#include    <Windows.h>

#include    <fileapi.h>

//_____________________________________________________________________________________________________________________________

class Cv_File
{
    int                     m_FileNo;
     
public:
    Cv_File( void)
        : m_FileNo( -1)
    {}

    ~Cv_File( void)
    { 
        Shut();
    } 

    bool    IsActive( void) const { return m_FileNo != -1; }  

    bool    Open( const char *name, bool readFlg)
    { 
        m_FileNo = open( name, readFlg ? O_RDONLY : ( O_WRONLY|O_CREAT|O_TRUNC), _S_IREAD | _S_IWRITE); 
        if ( m_FileNo == -1)
            return false;
        return true;
    }

    bool    Shut( void)
    {
        if (  m_FileNo != -1)
            close( m_FileNo); 
        m_FileNo = -1;
        return true;
    }
    
    uint32_t Read( void *buffer, uint32_t sz)
    {
        return read( m_FileNo, buffer, sz);
    }

    uint32_t Write( const void *buffer, uint32_t sz)
    {
        return write( m_FileNo, buffer, sz);
    }
    
};

//_____________________________________________________________________________________________________________________________


class Xd_InFileFlux
{
    enum {
        BSz = 64 * 1024
    };
    int                     m_FileNo;
    uint32_t                m_SzRead;
    uint32_t                m_Cur;
    uint32_t                m_LastRead;
    std::vector< uint8_t>   m_Buffer;

    bool LoadBuffer( void)
    {
        m_SzRead =  read( m_FileNo, &m_Buffer[ 0], uint32_t( m_Buffer.size()));
        m_Cur = 0;
        if (( m_SzRead == CV_UINT32_MAX) || ( m_SzRead == 0))
            return Shut();
        return true;
    } 
public:

    Xd_InFileFlux( uint32_t bufSz = BSz)
        : m_FileNo( -1), m_SzRead( 0), m_Cur( 0)
    {
        m_Buffer.resize( bufSz);
    }


    ~Xd_InFileFlux( void)
    { 
        Shut();
    } 

    bool    IsActive( void) const { return m_FileNo != -1; }  

    bool    Open( const char *name)
    { 

        m_FileNo = open( name, O_RDONLY); 
        if ( m_FileNo == -1)
            return false;
        return LoadBuffer();
    }

    bool    Shut( void)
    {
        if (  m_FileNo != -1)
            close( m_FileNo); 
        m_FileNo = -1;
        return false;
    }


    uint32_t    SzRead( void) const { return m_LastRead; }

    bool    SeekTo( uint64_t start)
    {
        CV_LSEEK( m_FileNo, start, SEEK_SET);
        if ( !LoadBuffer())
            return false; 
        return true;
    }

    bool        Restore( void *vptr, uint32_t sz)
    {    
        uint8_t     *ptr = ( uint8_t *) vptr;
        m_LastRead = 0;
        while ( sz)
        {
            uint32_t    left = m_SzRead -m_Cur;
            if ( !left)
            {
                if ( !LoadBuffer())
                    return false;
                left = m_SzRead -m_Cur;
            }
            uint32_t    szCopy = ( sz < left) ? sz : left;  
            memcpy( ptr, &m_Buffer[ m_Cur], szCopy);
            m_Cur += szCopy;
            ptr += szCopy;
            sz -= szCopy;
            m_LastRead += szCopy;
        }
        return true;    
    }

    Cv_CStr   ScanBufferTo( uint8_t chr)
    {
        uint8_t     *buf = &m_Buffer[ m_Cur];
        uint32_t    left = m_SzRead -m_Cur;

        for ( m_LastRead = 0; m_LastRead < left; ++m_LastRead, ++m_Cur)
        {
            if ( chr == buf[ m_LastRead])
                return Cv_CStr( ( const char *) buf, m_LastRead);
        }
        return Cv_CStr( ( const char *)  buf, m_LastRead);    
    }

    Cv_CStr     BufferUpTo( uint32_t sz)
    {
        uint8_t     *buf = &m_Buffer[ m_Cur];
        uint32_t    left = m_SzRead -m_Cur; 
        m_LastRead = ( sz < left) ? sz : left;
        m_Cur += m_LastRead;
        return Cv_CStr( ( const char *)  buf, m_LastRead);    
    }

    char NextChar()
    {
        if ( m_Cur >= m_SzRead)
            LoadBuffer();
        return m_Buffer[m_Cur++];
    }

    bool SkipSpaces()
    {
        char c;
        while (((c = NextChar()) == ' ' || c == '\n' || c == '\t') && c != '\0')
            ;
        if (c == '\0')
            return false;
        m_Cur--;
        return true;
    } 

    int ReadInt()
    {
        return (int)ReadLongLong();
    }

    long long ReadLongLong()
    {
        long long n = 0;
        int mul = 1;
        char c;
        if (!SkipSpaces())
            abort();
        c = NextChar();
        if (c == '+' || c == '-')mul = (c == '+') ? 1 : -1;
        else m_Cur--;
        do {
            c = NextChar();
            if (c >= '0' && c <= '9')n = n * 10 + (c - '0');
        } while (c >= '0' && c <= '9');
        return n * mul;
    }

    double ReadDouble()
    {
        double n = 0;
        int mul = 1, i = 0, dotpos = 0;
        char c;
        if (!SkipSpaces())
            abort();
        c = NextChar();
        if (c == '+' || c == '-')mul = (c == '+') ? 1 : -1;
        else m_Cur--;
        do {
            i++;
            c = NextChar();
            if (c >= '0' && c <= '9')n = n * 10.0 + (c - '0');
            if (c == '.')dotpos = i;
        } while ((c >= '0' && c <= '9') || c == '.');
        if (dotpos > 0)
            for (; dotpos < i - 1; dotpos++)n /= 10.0;
        return n * mul;
    }

    int ReadString(char*str)
    {
        if (!SkipSpaces())
            return str[0] = 0;
        char c;
        int l = 0;
        do {
            c = NextChar();
            str[l++] = c;
        } while (c != ' ' && c != '\n' && c != '\t' && c != '\0');
        str[--l] = '\0';
        return l;
    }

    char ReadChar() 
    {
        if (!SkipSpaces())
            return'\0';
        return NextChar();
    }
};