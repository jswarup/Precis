// cv_aid.h ____________________________________________________________________________________________________________________
#pragma once

#include    "cove/barn/cv_cexpr.h"
#include    "cove/silo/cv_cstr.h"
#include	"cove/silo/cv_tuple.h"
#include	"cove/flux/cv_cask.h"

//_____________________________________________________________________________________________________________________________

struct Cv_Aid
{
    //_____________________________________________________________________________________________________________________________

    enum 
    {
        BufferSz = 512,
    };

    //_____________________________________________________________________________________________________________________________

template < class Iterator, class X>
    static void  Set( Iterator b, Iterator e, const X &x) 
    { 
        for ( ; b != e; ++b) 
        	*b = x;
    }

    //_____________________________________________________________________________________________________________________________

template < class Iterator>
    static void  Init( Iterator b, Iterator e) 
    { 
        typedef typename Iterator::value_type ValueType;

        ValueType     x = Cv_CExpr::InitVal< ValueType>();
        for ( ; b != e; ++b) 
        	*b = x;
    }

template < class X>
    static void  MemSetZero( X *b, const X *e) 
    { 
        for ( ; b != e; ++b) 
        	*b = X( 0);
    }
    static bool                 FileExists( const char *fileName);
	
	static uint64_t				FileSize( FILE *fp);
    
    static std::string          Basename( const std::string &path);

	static std::string          Extension( const std::string &path);
    
    static uint32_t             BinCoeff( uint32_t n, uint32_t k);

    static Cv_CStr              TrimFuncName( const Cv_CStr &name);

    static std::string          TypenameCleanup( const Cv_CStr &typeIdName);

    static std::string          CPPTypeStringCleanup( const Cv_CStr &typeName) ;

    static char                 UpCase( char c) { return toupper( c);  }
    
    static void                 UpCase( std::string *pStr) { std::transform( pStr->begin(), pStr->end(), pStr->begin(), toupper);  }
    
    static Cv_Twin< Cv_CStr>    LSplit( const Cv_CStr &str, const Cv_CStr &ws);

    static Cv_Twin< Cv_CStr>    LSplit( const Cv_CStr &str, char c);
    
    static Cv_Twin< Cv_CStr>    RSplit( const Cv_CStr &str, char c);

    //_____________________________________________________________________________________________________________________________
    
template < class T>
    static auto ToString( const T &obj)
    {
        std::stringstream   sstrm;
        sstrm << obj; 
        return sstrm.str();
    }

    static uint32_t     Log2( uint64_t value)
    {
        unsigned long    index;
        if (!_BitScanReverse64(&index, value)) 
            return 0; 

        // add 1 if x is NOT a power of 2 (to do the ceil)
        return index + (value & (value - 1) ? 1 : 0);
    }
 
    static uint32_t     Log2( uint32_t value)
    {
        unsigned long    index;
        if (!_BitScanReverse64(&index, value)) 
            return 0; 

        // add 1 if x is NOT a power of 2 (to do the ceil)
        return index + (value & (value - 1) ? 1 : 0);
    }
    
template < typename TUInt>
    static uint32_t    NumOfDigits( TUInt v)
    {
        static const uint64_t   PowersOf10[] = { 0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 }; 
        uint32_t                t = ( Log2( v) + 1) * 1233 >> 12; // (use a log2 method from above)
        return 1 + t - (v < PowersOf10[t]);
    }


template <int count>
    static  void WriteChar(char* buf, uint64_t value)
    {
        uint64_t        div = value / 10;
        uint64_t        rem = value % 10;
        buf[ count - 1] = char( rem + '0');

        WriteChar< count - 1>(buf, div);
    }

template <>
    static void                 WriteChar<1>(char* buf, uint64_t value)  { buf[0] = char( '0' + value); }

    static uint32_t             WriteUInt1( char* data, uint64_t value); 

    static uint32_t             Write( char *data, uint64_t value) 
    { 
        static char buf[ 20];
        uint32_t    i = 20;
        do {
            buf[ --i] = (value % 10) + '0';
            value /= 10;
        } while ( value > 0);
        uint32_t    len = 20 -i + 1;
        memcpy( &buf[ i + 1], data, len);  
        return len;
    }

    static uint32_t             Write( char *data, const char *cstr) 
    {
        char    *begin = data;
        for ( ; *cstr; ++cstr, ++data)
            *data = *cstr;
        return uint32_t( data -begin);
    }

    static uint32_t             Write( char *data, const char chr) 
    {
        *data = chr;
        return 1;
    }

    static char                 HexToChar( int chr);

    static std::string          PrintBytes( const uint8_t *bytes, uint64_t len, uint32_t numBytesPerLine = CV_UINT32_MAX);

    static std::string          Format( const char *fmt, ...);

    static std::string          XmlEncode( const std::string &source);
    
template <typename T>
	static std::ostream			&OutStrm( std::ostream &out, T t) { return out << t; }

template <typename T, typename U, typename... Args>
	static std::ostream			&OutStrm( std::ostream &out, T t, U u, Args... args)
	{
		out << t << '_';
		return OutStrm( out, u, args...);
	}

template <typename... Args>
	static	std::string			ToStr( Args... args)
	{
		std::ostringstream	sstrm;
		OutStrm( sstrm, args...);
		return sstrm.str();
	}

    //_____________________________________________________________________________________________________________________________

template <typename T>
	static bool		ReadVec( std::vector< T> *pVec, const char *pathname) 
	{
		std::ifstream   is;
        is.open ( pathname, std::ios::binary ); 
        if ( !is)
            return false;

        // get length of file:
        is.seekg (0, std::ios::end);
        uint64_t 	length = is.tellg(); 
        pVec->resize( ( length + sizeof( T) -1)/sizeof( T)); 
        is.seekg (0, std::ios::beg); 
        // read data as a block:
        is.read ( ( char *) &pVec->at( 0), uint32_t( length));
        is.close();
        return true;
	}  

    //_____________________________________________________________________________________________________________________________

template < typename Spritz, typename T>
    static void Save( Spritz *spritz, const T &t) 
    {
        Cv_SerializeUtils::Save( spritz, t);
    }

template < typename Spritz, typename T>
    static void  Save( Spritz *spritz, T *t) 
    {
        Cv_SerializeUtils::Save( spritz, t);
    } 
	//_____________________________________________________________________________________________________________________________
};

//_____________________________________________________________________________________________________________________________

template< class X> 
struct Cv_PtrVector : public std::vector< X *> 
{};

//_____________________________________________________________________________________________________________________________

template < typename Piece>
struct  Cv_Enumerator
{
    virtual bool            MoveNext( void) = 0;
    virtual Piece           Current( void) const = 0;
    
    virtual ~Cv_Enumerator( void) {}         
};

//_____________________________________________________________________________________________________________________________

struct Cv_Dummy
{

};

//_____________________________________________________________________________________________________________________________

template < class Container, class Base = Cv_Dummy>
struct  Cv_StdEnumeration : public Base
{
    typedef typename Container::iterator    Iterator;
    typedef typename Container::value_type  Piece;

    Container       m_Container;
    Iterator        m_It;
    
    Cv_StdEnumeration( const Container &container)
        : m_Container( container)
    {
        m_It = m_Container.end();
    }

    Cv_StdEnumeration( Container &&container)
        : m_Container( container)
    {
        m_It = m_Container.end();
    }

    bool    MoveNext( void) 
    { 
        if ( m_It == m_Container.end())
            m_It = m_Container.begin();
        else
            ++m_It;
        return m_It != m_Container.end();
    }

    Piece   Current( void) const
    {
        return *m_It;
    }

};


//_____________________________________________________________________________________________________________________________

template < class Container >
struct  Cv_StdEnumerator : public Cv_StdEnumeration< Container, Cv_Enumerator< typename Container::value_type>  >
{
    typedef  Cv_StdEnumeration< Container, Cv_Enumerator< typename Container::value_type>  >        Base;
    Cv_StdEnumerator( Container &&container)
        : Base( container)
    {}
};
 

//_____________________________________________________________________________________________________________________________
