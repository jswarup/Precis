//_______________________________________________ cv_spritz.cpp _______________________________________________________________

#include	"cove/barn/cv_include.h"
#include	"cove/flux/cv_spritz.h"

#include <sys/types.h>
#include <sys/stat.h>


//_____________________________________________________________________________________________________________________________

void    Cv_Spritz::FillNull( uint64_t sz) 
{
    uint8_t     *buf = ( uint8_t *) alloca( sz); 
    memset( buf, 0, sz);
    Write(  buf, sz);
    return;
}
    
//_____________________________________________________________________________________________________________________________

Cv_FileSpritz::Cv_FileSpritz( const Cv_CStr &fname, Facet facet)
    : m_Fp( NULL), m_Facet( facet), m_Offset( 0)
{
    if ( m_Facet == ReadOnly)
        m_Fp = fopen( fname.CStr(), "rb");
    else if ( m_Facet & WriteTrim)
        m_Fp = fopen( fname.CStr(), "w+b");
}

//_____________________________________________________________________________________________________________________________

Cv_FileSpritz::~Cv_FileSpritz( void)
{
    if ( m_Fp)
        fclose( m_Fp);
}

//_____________________________________________________________________________________________________________________________
    
uint64_t        Cv_FileSpritz::SetOffset( uint64_t k)
{
    uint64_t    fSz = Size();
    if ( k < fSz)
    {
        m_Offset = k;
        CV_FSEEK( m_Fp, k, SEEK_SET);
    }
    else if (( m_Facet & WriteRead) || ( k == CV_UINT64_MAX))
    {
        m_Offset = fSz;
        CV_FSEEK( m_Fp, 0L, SEEK_END);
    } else 
        CV_ERROR_ASSERT( false)

    if (( m_Facet == ReadOnly) || ( k <= fSz) || ( k == CV_UINT64_MAX))
        return m_Offset;

    uint8_t     t[ 4096];
    t[ 0] = 1;
    uint64_t    sz = 4096;
    for ( uint64_t rem = k -fSz, wrSz = 0; rem; rem -=wrSz)
    {
        wrSz = rem > sz ? sz : rem;
        if ( t [ 0])
            memset( t, 0, wrSz);  
        fwrite( t, wrSz, 1, m_Fp);
    }
    m_Offset = k;
    return m_Offset;
}

//_____________________________________________________________________________________________________________________________
    
uint64_t    Cv_FileSpritz::EnsureSize( uint64_t sz)
{
    uint64_t    curOff = m_Offset;
    SetOffset( curOff + sz); 
    SetOffset( curOff);
    return curOff;
}

//_____________________________________________________________________________________________________________________________

bool    Cv_FileSpritz::Read( void *buf, uint64_t sz) 
{  
    m_Offset += sz; 
    return  fread( buf, sz, 1, m_Fp) == 1;  
}

//_____________________________________________________________________________________________________________________________

bool    Cv_FileSpritz::Write( const void *buf, uint64_t sz) 
{  
    if ( !( m_Facet & WriteRead))
        return false;
    m_Offset += sz;
    return  fwrite( buf, sz, 1, m_Fp) == 1;  
}

//_____________________________________________________________________________________________________________________________

void  Cv_FileSpritz::FlushAll( void)
{   
    fflush( m_Fp);
    return;
}

//_____________________________________________________________________________________________________________________________

uint64_t    Cv_FileSpritz::Size( void) const 
{   
    int     res = CV_FSEEK( m_Fp, 0L, SEEK_END );                                    // go to the end of file
    CV_ERROR_ASSERT( res == 0)
    return ( uint64_t ) CV_FTELL( m_Fp);}


//_____________________________________________________________________________________________________________________________

uint64_t  Cv_FileSpritz::SaveBuffer( uint64_t fOff, const void *buf, uint64_t sz)
{
    fOff = SetOffset( fOff); 
    size_t      nWr = fwrite( buf, sz, 1,  m_Fp);             // write the page content.
    CV_ERROR_ASSERT( nWr == 1)
    return fOff;
}

//_____________________________________________________________________________________________________________________________

void    Cv_FileSpritz::RestoreBuffer( uint64_t fOff, void *buf, uint64_t sz)
{
    fOff = SetOffset( fOff); 
	size_t    nRd = fread( buf, sz, 1, m_Fp);
    CV_ERROR_ASSERT( nRd == 1)
    return;
}

//_____________________________________________________________________________________________________________________________

Cv_ValidationSpritz::~Cv_ValidationSpritz( void)
{
}

//_____________________________________________________________________________________________________________________________

bool    Cv_ValidationSpritz::Read( void *buf, uint64_t sz)
{
    uint64_t                offset = m_WorkSpritz->Offset();
    m_TestSpritz->SetOffset( offset);
    bool                    res = m_WorkSpritz->Read( buf, sz);
    std::vector< uint8_t>   bench( sz);
    bool                    res2 = m_TestSpritz->Read( &bench[ 0], sz);
    int                     res3 = memcmp( buf, &bench[ 0], sz);
    CV_ERROR_ASSERT( ( res == res2) && ( res3 == 0))
    return res;
}

//_____________________________________________________________________________________________________________________________

bool    Cv_ValidationSpritz::Write( const void *buf, uint64_t sz)
{
    uint64_t                offset = m_WorkSpritz->Offset();
    m_TestSpritz->SetOffset( offset);
    std::vector< uint8_t>   bench( sz);
    bool                    res2 = m_TestSpritz->Read( &bench[ 0], sz);
    int                     res3 = memcmp( buf, &bench[ 0], sz);
    bool                    res = m_WorkSpritz->Write( buf, sz);
    CV_ERROR_ASSERT( ( res == res2) && ( res3 == 0))
    return res;
}



//_____________________________________________________________________________________________________________________________

