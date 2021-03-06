// cv_include.h _____________________________________________________________________________________________________________

#pragma once 
#pragma warning(disable: 4100)


//_____________________________________________________________________________________________________________________________

#ifdef _MSC_VER
#define     CV_WINDOWS
#define		PR_VISUALSTUDIO 
#endif

#define CV_TRACEDBG

#include    <type_traits>
#include    <list>
#include    <stack>
#include    <string>
#include    <vector>
#include    <bitset>
#include    <set>
#include    <map>
#include    <algorithm>
#include    <iostream>
#include    <fstream>
#include    <sstream>
#include    <typeinfo>
#include    <memory>
#include    <numeric>
#include    <valarray>
#include    <stdexcept>
#include    <limits>
#include    <functional>
#include    <atomic>
#include    <thread>
#include    <cstring>
#include    <stdarg.h> 
#include    <chrono>
#include    <ctime>
#include    <cmath>
#include    <array>

#ifdef CV_WINDOWS
#include    <io.h>
#include    <intrin.h>
#else
#include    <unistd.h> 
#include    <x86intrin.h>
#endif
 
#define     CV_UINT8_MAX        uint8_t( -1)
#define     CV_UINT16_MAX       uint16_t( -1)
#define     CV_UINT32_MAX       uint32_t( -1)
#define     CV_UINT64_MAX       uint64_t( -1)
#define     CV_UINT32_SUBMAX    ( uint32_t( -1) -1)
        
#define     CV_INT8_MIN         int8_t( 0x80)    
#define     CV_INT8_MAX         int8_t( 0x7F)
         
#define     CV_INT16_MIN        int16_t( 0x8000)
#define     CV_INT16_MAX        int16_t( 0x7FFF) 
         
#define     CV_INT32_MIN        int32_t( 0x80000000)
#define     CV_INT32_MAX        int32_t( 0x7FFFFFFF)
        
#define     CV_INT64_MIN        int64_t( 0x8000000000000000)
#define     CV_INT64_MAX        int64_t( 0x7FFFFFFFFFFFFFFF)

#define     CV_LOWMASK( x)	    (( uint64_t (0x1) << ( x)) -1)

//_____________________________________________________________________________________________________________________________

template < typename X>
struct  Cv_Couple : public std::tuple< X, X> 
{
    Cv_Couple( void) {}
    
    Cv_Couple( const std::tuple< X, X> &data)
        : std::tuple< X, X>( data)
    {}
    
    Cv_Couple( std::tuple< X, X> &&data)
        : std::tuple< X, X>( data)
    {};
};

//_____________________________________________________________________________________________________________________________

#define Cv_MAKESTR(s)       #s

#define Cv_CONCAT(s,t)      Cv_MAKESTR(s##t)

template < typename T>
uint8_t cv_PopCount( T t);

#ifdef CV_WINDOWS

#pragma warning( disable :4355)
#pragma warning( disable :4996)

#define CV_FUNCNAME()       Cv_CStr( __FUNCTION__)
#define CV_LSEEK            _lseeki64
#define CV_FSEEK            _fseeki64
#define CV_FTELL            _ftelli64 
#define CV_STRCASECMP       stricmp
#define CV_STRNCASECMP      strnicmp  

#define CV_PREFETCH_CACHE( Addr)               {  _m_prefetch( Addr);  }

template <>
inline uint8_t cv_PopCount< uint8_t>( uint8_t t) { return uint8_t( __popcnt16( t)); } 

template <>
inline uint8_t cv_PopCount< uint64_t>( uint64_t t) { return uint8_t( __popcnt64( t)); }

#else

#define CV_FUNCNAME()       Cv_Aid::TrimFuncName( __PRETTY_FUNCTION__)
#define CV_LSEEK            lseek64
#define CV_FSEEK            fseek
#define CV_FTELL            ftell      
#define CV_STRCASECMP       strcasecmp 
#define CV_STRNCASECMP      strncasecmp   

#define Cv_PREFETCH_CACHE( Addr)               {  _builtin_prefetch( Addr);  

template <>
inline uint8_t cv_PopCount< uint8_t>( uint8_t t) { return uint8_t( __builtin_popcount ( t)); } 

template <>
inline uint8_t cv_PopCount< uint64_t>( uint64_t t) 
            { static_assert( sizeof( long) == sizeof( uint64_t), "long is not 64-bit"); return uint8_t( __builtin_popcountl( t)); }

#endif


template < typename T>
T       *cv_pcast( const void *data) { return static_cast< T *>( const_cast< void *>( data)); }

//_____________________________________________________________________________________________________________________________

#define   SELF  (*this) 

#define CV_CACHELINE_SIZE   64
    
//_____________________________________________________________________________________________________________________________

class Cv_ErrorMgr 
{
public:
    static bool    ErrorTrap( const char *, uint32_t l);
    
    static void    Print( const char *fmt, ...)
    {
        char            buf[ 1024];
        va_list        arg;
        va_start (arg, fmt);   
        vsprintf( buf, fmt, arg);
        va_end ( arg);  
        std::string     eMsg;
        eMsg += "Error: ";
        eMsg += buf;
        printf( "%s\n", eMsg.c_str());
        return; 
    }
};


#define CV_THROW_EXCEPT( msg)                                                               \
    {                                                                                       \
    }

#define CV_ERROR_EXCEPT( x, msg)                                                            \
    {                                                                                       \
        if ( ! ( x))                                                                        \
        {                                                                                   \
            Cv_ErrorMgr::Print msg;                                                         \
        }                                                                                   \
    }



#define CV_DEBUG_ASSERT( x) {}

#define CV_ERROR_ASSERT( x)                                                                 \
{                                                                                           \
    if ( ! ( x))                                                                            \
        Cv_ErrorMgr::ErrorTrap(__FILE__, __LINE__);                                         \
}




#define CV_ERROR_RETURN()                                                                   \
      return  Cv_ErrorMgr::ErrorTrap(__FILE__, __LINE__);                                   \
      


#ifdef CV_SANITY 

#define CV_SANITY_ASSERT( x )                                                                \
{                                                                                           \
    if ( ! ( x))                                                                            \
        Cv_ErrorMgr::ErrorTrap(__FILE__, __LINE__);                                         \
}
   
#else
   
#define CV_SANITY_ASSERT( x) {}

#endif
//_____________________________________________________________________________________________________________________________
