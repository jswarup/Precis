//_________________________________________ cv_include.h ____________________________________________________________________
#pragma once
 


#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#define strcasecmp      stricmp
#define strncasecmp     strnicmp 
#endif

//_____________________________________________________________________________________________________________________________


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

#define Hv_MAKESTR(s)       #s

#define Hv_CONCAT(s,t)      Hv_MAKESTR(s##t)

#ifdef _MSC_VER

#pragma warning( disable :4355)
#pragma warning( disable :4996)

#define Hv_FUNCNAME()       Cv_CStr( __FUNCTION__)
#define Hv_FSEEK            _fseeki64
#define Hv_FTELL            _ftelli64

#else

#define Hv_FUNCNAME()       Cv_Aid::TrimFuncName( __PRETTY_FUNCTION__)
#define Hv_FSEEK            fseek
#define Hv_FTELL            ftell

#endif


template < typename T>
T       *cv_pcast( const void *data) { return static_cast< T *>( const_cast< void *>( data)); }

//_____________________________________________________________________________________________________________________________

#define   SELF  (*this) 

#define Hv_CACHELINE_SIZE 64
#define Hv_PREFETCH_CACHE( Addr)               {  _builtin_prefetch( Addr);  
    
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
