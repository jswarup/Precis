//___________________________________________ cv_aid.cpp _____________________________________________________________ 

#include    "cove/barn/cv_include.h"
#include 	"cove/barn/cv_aid.h"
#include    <regex>
#include    <array>
#include    <cstdarg>

//_____________________________________________________________________________________________________________________________
 
bool Cv_Aid::FileExists(const char *fileName)
{
	if ( !fileName)
		return false;

	FILE    *fp = fopen( fileName, "r");
	if (!fp)
		return false;
	fclose( fp);
	return true;
}

//_____________________________________________________________________________________________________________________________

uint64_t    Cv_Aid::FileSize( FILE *fp)
{
#if defined( PR_VISUALSTUDIO) 
	uint64_t    pos = _ftelli64( fp);
	_fseeki64( fp, 0, SEEK_END );
	uint64_t    sz = _ftelli64( fp );
	_fseeki64( fp, pos, SEEK_SET );
#else
	uint64_t    pos = ftell( fp);
	fseek( fp, 0, SEEK_END );
	uint64_t    sz = ftell( fp );
	fseek( fp, pos, SEEK_SET );
#endif
	return sz;
} 

//_____________________________________________________________________________________________________________________________

std::string  Cv_Aid::Basename( const std::string &path)
{
	std::string     filename = path;
	size_t          pos = path.rfind("/");
	if ( pos == size_t( -1) )
		pos = filename.rfind("\\");
	if ( pos != size_t( -1) )
		filename.erase(filename.begin(), filename.begin() + pos + 1);
	return filename;
}

//_____________________________________________________________________________________________________________________________

std::string  Cv_Aid::Extension( const std::string &path)
{
	std::string     filename = path;
	size_t          pos = path.rfind(".");
	if ( pos ==  size_t( -1) )
		return std::string();

	return std::string( filename.begin() + pos + 1, filename.end());
}

//_____________________________________________________________________________________________________________________________

uint32_t Cv_Aid::BinCoeff(uint32_t n, uint32_t k)
{
    uint32_t    c = 1;
    if ( k > ( n - k))
        k = n - k;

    for ( uint32_t i = 1; i <= k; i++, n--)
    {
        if ( c / i > CV_UINT32_MAX / n) 
            return 0;                       // 0 on overflow 
        c = c / i * n + c%i * n / i;            //split c*n/i into (c/i*i + c%i)*n/i 
    }
    return c;
}


//_____________________________________________________________________________________________________________________________

Cv_Twin< Cv_CStr>   Cv_Aid::LSplit( const Cv_CStr &str, const Cv_CStr &ws)
{
    uint32_t    i = 0;
    for (  ; i < str.Size(); ++i)
        if ( ws.Locate( str[ i]) != CV_UINT32_MAX)
            break;       
    Cv_Twin< Cv_CStr>   ct;
    ct.SetFirst( Cv_CStr( &str[ 0], i));
    if ( i < str.Size())
        ct.SetSecond( Cv_CStr( &str[ i], str.Size() -i));
    return ct; 
}

//_____________________________________________________________________________________________________________________________

Cv_Twin< Cv_CStr>   Cv_Aid::LSplit( const Cv_CStr &str, char c)
{
    uint32_t    i = 0;
    for (  ; i < str.Size(); ++i)
        if ( str[ i] == c)
            break;       
    Cv_Twin< Cv_CStr>   ct;
    ct.SetFirst( Cv_CStr( &str[ 0], i));
    if ( i < str.Size())
        ct.SetSecond( Cv_CStr( &str[ i], str.Size() -i));
    return ct; 
}

//_____________________________________________________________________________________________________________________________

Cv_Twin< Cv_CStr>   Cv_Aid::RSplit( const Cv_CStr &str, char c)
{
    uint32_t    i = str.Size();
    for (  ; i != 0; --i)
        if ( str[ i -1] == c)
            break;       
    Cv_Twin< Cv_CStr>   ct;
    ct.SetSecond( Cv_CStr( &str[ i], str.Size() -i));
    if ( i != 0)
        ct.SetFirst( Cv_CStr( &str[ 0], i));
    return ct; 
}

//_____________________________________________________________________________________________________________________________

Cv_CStr Cv_Aid::TrimFuncName( const Cv_CStr &name)
{
    Cv_Twin< Cv_CStr>   parts = LSplit(  name, '(');
    const Cv_CStr       &str = parts.First();
    uint32_t            i = str.Size();
    uint32_t            nAng = 0;
    for (  ; i != 0; --i)
    {
        if ( !nAng && ( str[ i -1] == ' '))
            break;       
        if ( str[ i -1] == '>')
            ++nAng; 
        if ( str[ i -1] == '<')
        {
            CV_ERROR_ASSERT( nAng)
            --nAng;        
        }
    }
    return  Cv_CStr( &str[ i], str.Size() -i);
}

//_____________________________________________________________________________________________________________________________

std::string  Cv_Aid::TypenameCleanup( const Cv_CStr &typeIdName) 
{
    using   RgxData = std::pair< std::regex, std::string>      ;
    
    static const std::array< RgxData, 7>   subs{{                                                           
        RgxData( std::regex("\\b(class|struct|enum|union) "), ""),              // Remove unwanted keywords and following space. (\b is word boundary.)
        RgxData( std::regex("[`(]anonymous namespace[')]"), "(anonymous)"),     // Tidy up anonymous namespace.
        RgxData( std::regex("\\b__int64\\b"), "long"),                     // Replace Microsoft __int64 with long long.
//        RgxData( std::regex("(\\w) (\\w)"), "$1!$2"),                           // Temporarily replace spaces we want to keep with "!". (\w is alphanumeric or underscore.)
        RgxData( std::regex(" "), ""),                                          // Delete unwanted spaces.
        RgxData( std::regex("\\b__[[:alnum:]_]+::"), ""),                       // Some compilers throw in extra namespaces like "__1" or "__cxx11".Delete them.
        RgxData( std::regex("!"), " "),                                         // Restore wanted spaces.
        RgxData( std::regex("\\bstd::basic_string<char,std::char_traits<char>,"  
                                "std::allocator<char>>"), "std::string")        // Recognize std::string's full name and abbreviate.
    }};

#ifdef __GNUG__
/*
    int             status = -100; 
    char            *ret = abi::__cxa_demangle( typeIdName.CStr(), NULL, NULL, &status);
    std::string     demangled( ret);
    delete ret;
  */  std::string     demangled( typeIdName);
#else
    std::string     demangled( typeIdName);
#endif

    for ( const auto& sp : subs) 
        demangled = std::regex_replace(demangled, sp.first, sp.second);
 
    return demangled;
}

//_____________________________________________________________________________________________________________________________

std::string  Cv_Aid::CPPTypeStringCleanup( const Cv_CStr &typeName) 
{
    std::string     typeStr = typeName;
    typeStr = std::regex_replace( typeStr, std::regex("\\["), "<");
    typeStr = std::regex_replace( typeStr, std::regex("\\]"), ">");
    return typeStr;    
}


//_____________________________________________________________________________________________________________________________

char    Cv_Aid::HexToChar( int chr)
{
	static char		cmap[ 16];
	static char		*cmapPtr = NULL;

	if ( cmapPtr)
		return cmapPtr[ chr];
	cmapPtr = cmap;

	cmap[  0] = '0';
	cmap[  1] = '1';
	cmap[  2] = '2';
	cmap[  3] = '3';
	cmap[  4] = '4';
	cmap[  5] = '5';
	cmap[  6] = '6';
	cmap[  7] = '7';
	cmap[  8] = '8';
	cmap[  9] = '9';
	cmap[ 10] = 'a';
	cmap[ 11] = 'b';
	cmap[ 12] = 'c';
	cmap[ 13] = 'd';
	cmap[ 14] = 'e';
	cmap[ 15] = 'f';

	return cmapPtr[ chr];
}

//_____________________________________________________________________________________________________________________________

std::string Cv_Aid::PrintBytes( const uint8_t *bytes, uint64_t len, uint32_t numBytesPerLine)
{
    std::stringstream   sstrm;
	for ( uint64_t i = 0; i < len; ++i)
    {
        if ( i)
        {
		    if ( i % numBytesPerLine)
			    sstrm << ' ';
            else
                sstrm << '\n';
        }
        int			a = bytes[ i] & 0xf;
		int			b = ( bytes[ i] >> 4) & 0xf;
		sstrm <<  HexToChar( b) << HexToChar( a);
	}
	return sstrm.str();
}

//_____________________________________________________________________________________________________________________________

std::string Cv_Aid::Format( const char *fmt, ...)
{
    char                        buffer[ BufferSz];
    va_list     vl;
    va_start( vl, fmt);
    int                         nsize = vsnprintf( buffer, BufferSz, fmt, vl);
    CV_ERROR_ASSERT( nsize)
    va_end(vl);
    return std::string( &buffer[ 0], &buffer[ nsize]);
}
 

///-------------------------------------------------------------------------------------------------
// encodes special characters in xml

std::string Cv_Aid::XmlEncode( const std::string &source) 
{
    std::string     dest;
    for ( std::string::const_iterator it = source.begin(); it != source.end(); ++it)
        switch (*it)
        {
        case '&': dest +=  "&amp;"; break;
        case '<': dest +=  "&lt;"; break;
        case '>': dest +=  "&gt;"; break;
        case '"': dest +=  "&quot;"; break;
        case '\'': dest +=  "&apos;"; break;
        case ' ': dest += "&#160;"; break;
            /*
            case 'ß': dest += "&#223;"; break;
            case 'ä': dest += "&#228;"; break;
            case 'ö': dest += "&#246;"; break;
            case 'ü': dest += "&#252;"; break;
            case 'Ä': dest += "&#196;"; break;
            case 'Ö': dest += "&#214;"; break;
            case 'Ü': dest += "&#220;"; break;
            */
        default :
            dest += (*it);
        }
    return dest;
}
 
//_____________________________________________________________________________________________________________________________


