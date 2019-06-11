// cv_cexpr.h ______________________________________________________________________________________________________________
#pragma once

//_____________________________________________________________________________________________________________________________

class Cv_CExpr
{
public:
template < class X>
    static bool                 TestAspect( X x, X y) { return (( x & y) == y); }

template < uint32_t Size>
    static constexpr uint32_t   AlignTo( uint32_t sz)  { return (( sz + Size -1) / Size) * Size; }

template < class X>    
    static constexpr X          Pow2( X bits) { return bits == 0 ? 1 : 2 * Cv_CExpr::Pow2( bits -1); }

template < class X>
    static constexpr auto       InitVal( void) { return X(); }

template < int>
    static constexpr int        InitVal( void) { return 0; }

template < uint32_t>
    static constexpr uint32_t   InitVal( void) { return 0; }

    static constexpr uint64_t	LowMask( uint64_t x) {  return (( ( uint64_t(0x1) << ( x -1)) -1) << 1) | 1; }

template < class X>    
    static constexpr const X    &Min( const X &x1, const X &x2) { return x1 < x2 ? x1 : x2; }
 
template<typename T>
    constexpr auto IsFinalizeable(int) -> decltype(std::declval<T>().Finalize, bool())
    {
        return true;
    }
  
template<typename T> constexpr bool IsFinalizeable(...)
    {
        return false;
    }

};

//_____________________________________________________________________________________________________________________________

struct Cv_TypeEngage
{
template < typename T> 
    struct  Exist
    { 
        typedef void        Note; 
    };

template < typename T1, typename T2>
	struct  Exist2
	{
		typedef void        Note;
	};

    //_____________________________________________________________________________________________________________________________

    template < typename T, typename = void> 
    struct  Not
    {  
    };

    template < typename T> 
    struct  Not< T, typename Exist< T>::Note> 
    {
        typedef void        Note; 
    };

    //_____________________________________________________________________________________________________________________________

template < typename T1, typename T2> 
    struct Same
    { 
    };


template < typename T1> 
    struct Same< T1, T1>
    { 
        typedef void      Note; 
    };

    //_____________________________________________________________________________________________________________________________

template < typename T1, typename T2> 
    struct NotSame
    { 
        typedef void      Note; 
    };


template < typename T1> 
    struct NotSame< T1, T1>
    { 
    };

    //_____________________________________________________________________________________________________________________________

template < typename T, typename = void> 
    struct Stream
    { 
        typedef std::false_type  Note;
    };
       
template < typename T>
    struct Stream< T, typename Exist< decltype( operator<<( *( std::ostream *) nullptr, *( const T *) nullptr)) >::Type> 
    { 
        typedef std::true_type  Note;
    };

    //_____________________________________________________________________________________________________________________________

template < typename T>
    static auto Dump( T *shard, std::ostream &ostr, int k) ->   decltype( std::declval<T>().Dump( std::declval<std::ostream>()), bool())
    {         
        shard->Dump( ostr);
        return true;
    }

 template < typename T>
    static auto Dump( T *shard, std::ostream &ostr, ...) -> bool
    {         
        return false;
    }


template<class T, class EqualTo>
    struct HasStreamInsImpl
    {
        template<class U, class V>
        static auto Test(U*) -> decltype(std::declval<U>() << std::declval<V>());
        template<typename, typename>
        static auto Test(...) -> void;

        using Note = typename Same<bool, decltype(Test<T, EqualTo>(0))>::Note;
    };

template<class T, class EqualTo = T>
    struct HasStreamIns : HasStreamInsImpl<T, EqualTo>::Note {};
};


//_____________________________________________________________________________________________________________________________

template < typename T, typename = void>
struct Cv_TrivialCopy
{
};

template < typename T>
struct Cv_TrivialCopy< T, typename std::enable_if< std::is_arithmetic<T>::value>::type> 
{
    typedef void        Note;
};

template < typename T>
struct Cv_TrivialCopy< T, typename T::Copiable> 
{
    typedef void        Note;
};

//_____________________________________________________________________________________________________________________________

template < typename T, typename = void>
struct Cv_PointerType
{
};

template < typename T>
struct Cv_PointerType< T, typename std::enable_if< std::is_pointer<T>::value>::type >
{
    typedef void        Note;
};


//_____________________________________________________________________________________________________________________________

template < uint32_t  Sz>
struct Cv_For : public Cv_For< Sz-1>
{
    typedef Cv_For< Sz-1>    Base; 

template < typename Lambda, typename... Args>
    static void    RunAll( const Lambda &lambda,  const Args&... args)  
    {
        Base::RunAll( lambda, args...);
        lambda( Sz -1, args...); 
    }
};

//_____________________________________________________________________________________________________________________________

template <>
struct Cv_For< 1>
{   
    
template < typename Lambda, typename... Args>
    static void    RunAll( const Lambda &lambda,  const Args&... args)  
    {
        lambda( 0, args...); 
    }
};

//_____________________________________________________________________________________________________________________________
