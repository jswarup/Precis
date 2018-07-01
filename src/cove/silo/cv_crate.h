//_______________________________________________ cv_crate.h _______________________________________________________________
#pragma once
 
#include    "cove/silo/cv_repos.h"
#include    "cove/barn/cv_cexpr.h"

//_____________________________________________________________________________________________________________________________

class  Cv_CrateEntry : public Cv_ReposEntry
{
public:
    uint32_t                         m_Type; 

public:
    Cv_CrateEntry( uint32_t id = CV_UINT32_MAX)
        :  Cv_ReposEntry( id), m_Type( 0)
    {} 
 
 template <  typename Crate,  typename Lambda, typename... Args>
    auto    Operate(  Lambda lambda,  Args&... args)  
    {
        return Crate::Operate( static_cast< typename Crate::Entry *>( this), lambda, args...);
    } 

};  

//_____________________________________________________________________________________________________________________________


template < typename ValType>
struct Cv_CrateLambdaAccum
{
}; 


//_____________________________________________________________________________________________________________________________

template <>
struct Cv_CrateLambdaAccum< bool>
{
    bool    m_Value;

    Cv_CrateLambdaAccum< bool>( bool v = true)
        :  m_Value( v)
    {}

    operator bool( void) { return  m_Value; }

    Cv_CrateLambdaAccum &Accumulate( const Cv_CrateLambdaAccum &inp)
    {
        m_Value = m_Value && inp.m_Value;
        return *this;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T, typename... Rest>
struct Cv_Crate : public Cv_Crate< Rest...>
{   
    typedef Cv_Crate< T, Rest...>       Crate;
    typedef Cv_Crate< Rest...>          CrateBase;
    typedef T                           Elem;
    typedef typename CrateBase::Entry   Entry;
    
    enum {
        Sz = CrateBase::Sz +1,
    };
      
    Cv_Crate( void) 
    {
        
    }     

template <typename X, typename std::enable_if< std ::is_base_of< T, X>::value, void>::type * = nullptr>
    void AssignIndex( X *obj)
    {
        obj->m_Type =  Sz;
    } 

template < typename X, typename std::enable_if< !std ::is_base_of< T, X>::value, void>::type * = nullptr>
    void AssignIndex( X *obj)
    {
        CrateBase::AssignIndex( obj);
    } 
  
template <  typename Lambda, typename... Args>
    static auto    Operate( Entry *entry, Lambda &lambda,  Args&... args)  
    {
        if ( entry->m_Type ==  Sz)
            return lambda( static_cast< Elem *>( entry), args...); 
        return CrateBase::Operate( entry, lambda, args...);
    }
}; 

//_____________________________________________________________________________________________________________________________

template<typename T, typename=void>
struct Cv_CrateT
{ 

    enum {
        Sz = 1,
    }; 

    typedef  T      Entry;
    typedef T       Elem;

template < typename X = void>    
    void AssignIndex( X *obj)
    { 
        obj->m_Type =  Sz;
    }

template <  typename Lambda, typename... Args>
    static auto    Operate( Entry *entry, Lambda &lambda,  Args&... args)  
    {
        return lambda( static_cast< Elem *>( entry), args...); 
    }
}; 

template<typename T>
struct  Cv_CrateT<T, typename  Cv_TypeEngage::Exist< typename T::Elem>::Note> : public T
{
};

//_____________________________________________________________________________________________________________________________

template < typename T >
struct   Cv_Crate< T> : Cv_CrateT< T>
{ 
}; 

//_____________________________________________________________________________________________________________________________

template < typename Crate>
class Cv_CrateRepos  : public Crate
{ 
public: 
    typedef typename Crate::Entry                           Entry; 

protected:
    Cv_Repos< Entry>                                        *m_Repos;

   
public: 
    Cv_CrateRepos( Cv_Repos< Entry> *repos)
        : m_Repos( repos)
    {}

    void Clear( void)
    {
        OperateAll( []( auto x) { delete x; return true; }); 
        m_Repos->m_Elems.clear();
    }

template<  class Object>
    void    Store( Object *x)
    {
        Crate::AssignIndex( x);        
        uint32_t    k = m_Repos->Size();
        m_Repos->SetAt( k, x);
        return;
    }
 
template < typename Lambda, typename... Args>
    auto    OperateAll(  Lambda lambda,  Args&... args)  
    {   
        typedef Cv_CrateLambdaAccum< decltype( lambda(  static_cast<Entry *>( nullptr), args...))>     Accum;
        Accum                                               accum;
        for ( uint32_t i = 0; i < m_Repos->Size(); ++i)
        {
            Entry     *si = m_Repos->m_Elems[ i]; 
            if ( !si)
                continue; 
            if ( !accum.Accumulate( Crate::Operate( si, lambda, args...)))
                return accum;
        }
        return accum;
    }

    struct   Constructor 
    {  
        Cv_CrateRepos                   *m_Crate;
        std::map< void *, Entry *>      m_CnstrMap;

        Constructor( Cv_CrateRepos  *crate) 
            : m_Crate( crate)
        {}

    template < typename Node>    
        Entry     *FetchSynTree( Node *node)
        {
            typedef typename Node::SynElem      SynItem;

            auto        res  = m_CnstrMap.emplace( node, ( Entry *) NULL); 
            if ( !res.second)
                return static_cast< SynItem *>( res.first->second); 
            SynItem     *synItem = new SynItem();
            auto        item = synItem->Setup( node, this);
            if ( item != static_cast< Entry *>( synItem))
                delete synItem;
            m_Crate->Store( synItem);
            res.first->second = item;
            return item;
        }     
    };
};  

//_____________________________________________________________________________________________________________________________

template < typename Crate>
class Cv_CrateStack : public Crate 
{ 
public: 
    typedef typename Crate::Entry                           Entry; 

protected:
    Cv_Stack< Entry>        *m_Stack;

public: 
    Cv_CrateStack( Cv_Stack< Entry> *stack)
        : m_Stack( stack)
    {}

template<  class Object>
    void    Store( Object *x)
    {
        Crate::AssignIndex( x);  
        m_Stack->Push( x);
        return;
    }

 
template < typename Lambda, typename... Args>
    auto    OperateAll(  Lambda &lambda,  Args&... args)  
    {   
        typedef Cv_CrateLambdaAccum< decltype( lambda( static_cast<Entry *>( nullptr), args...))>     Accum;
        Accum                                               accum;
        for ( Entry    *si = m_Stack->Top(); si; si = si->GetBelow())
        {  
            if ( !accum.Accumulate( Crate::Operate( si, lambda, args...)))
                return accum;
        }
        return accum;
    }
};

//_____________________________________________________________________________________________________________________________

