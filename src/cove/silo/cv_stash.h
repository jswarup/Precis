// cv_stash.h _________________________________________________________________________________________________________________
#pragma once
 
#include    "cove/barn/cv_cexpr.h" 

//_____________________________________________________________________________________________________________________________

template < template<uint32_t> typename Param, uint32_t N, uint32_t Last = 0>
struct Cv_Stash : public Cv_Stash< Param, N -1, Last>
{  
    typedef typename Param< N>          Elem;
    typedef Cv_Stash< Param, N, Last>         Stash;
    typedef Cv_Stash< Param, N -1, Last>      StashBase;

    enum {
        Sz = N,
    };

template <  typename Lambda, typename... Args>
    static auto    Operate( void *entry, uint32_t typeStor, Lambda &lambda,  Args&... args)  
    {
        switch ( typeStor)
        {
            case Sz : return lambda( static_cast< Elem *>( entry), args...); 
            default : return StashBase::Operate( entry, typeStor, lambda, args...);
        }
    } 

    static uint32_t SizeOf( uint32_t typeStor)
    {
        switch ( typeStor)
        {
            case Sz : return sizeof( Elem); 
            default : return StashBase::SizeOf( typeStor);
        }
    }  

    void    Init( uint32_t typeStor, void *p, uint32_t sz)
    {
        switch ( typeStor)
        {
            case Sz : 
                    for ( uint32_t i = 0; i < sz; ++i)
                        new (( (uint8_t *) p) + i * sizeof (Elem)) Elem(); 
            default : return StashBase::Init( typeStor, p, sz);
        }
    }
};

template < template< uint32_t> typename Param, uint32_t Last>
struct Cv_Stash< Param, Last, Last>
{  
    typedef typename Param< Last>          Elem; 
    typedef Cv_Stash< Param, Last>         Stash;
    
    enum {
        Sz = 0,
    };


template <  typename Lambda, typename... Args>
    static auto    Operate( void *entry, uint32_t typeStor, Lambda &lambda,  Args&... args)  
    {
        return lambda( static_cast< Elem *>( entry), args...); 
    }

    static uint32_t SizeOf( uint32_t typeStor)
    {
        return sizeof( Elem); 
    }
 
    void    Init( uint32_t typeStor, void *p, uint32_t sz)
    {
        for ( uint32_t i = 0; i < sz; ++i)
            new (( (uint8_t *) p) + i * sizeof (Elem)) Elem(); 
    }
};

//_____________________________________________________________________________________________________________________________
