// cv_crate.h ________________________________________________________________________________________________________________
#pragma once

#include    "cove/silo/cv_repos.h"
#include    "cove/barn/cv_cexpr.h"

//_____________________________________________________________________________________________________________________________

class Cv_CrateId
{
	uint32_t	m_Id;
	uint8_t		m_Type;
	uint8_t		m_Use; 

public:
	Cv_CrateId( uint32_t id = 0, uint8_t type = 0)
		: m_Id( id), m_Type( type), m_Use( 1)
	{}

	uint64_t	Id( void) const { return m_Id; }
	void		SetId( uint32_t	id) { m_Id = id; }

	uint32_t	Type( void) const { return m_Type; }
	void		SetType( uint8_t type) { m_Type = type; }

	uint8_t		RefCount( void) const { return m_Use; }
	uint8_t		RaiseRef( void)  { return ++m_Use; }
	uint8_t		lowerRef( void)  { return --m_Use; }
};

//_____________________________________________________________________________________________________________________________

class  Cv_CrateEntry  : public Cv_CrateId
{ 
public:
    Cv_CrateEntry( uint32_t id = 0, uint8_t type = 0)
        :  Cv_CrateId( id, type)
    {}  

	const char      *GetName( void) const { return ""; }

 template <  typename Crate,  typename Lambda, typename... Args>
    auto    Operate(  Lambda lambda,  Args&... args)  
    {
        return Crate::OperateOn( static_cast< typename Crate::Entry *>( this), lambda, args...);
    } 
};  

//_____________________________________________________________________________________________________________________________

template < typename ValType>
struct Cv_CrateLambdaAccum
{}; 

//_____________________________________________________________________________________________________________________________

template <>
struct Cv_CrateLambdaAccum< bool>
{
    bool    m_Value;

    Cv_CrateLambdaAccum( bool v = true)
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
    {}     

template <typename X, typename std::enable_if< std::is_base_of< T, X>::value, void>::type * = nullptr>
    uint8_t AssignIndex( X *obj)
    {
		obj->SetType( Sz);
        return Sz;
    } 

template < typename X, typename std::enable_if< !std::is_base_of< T, X>::value, void>::type * = nullptr>
	uint8_t AssignIndex( X *obj)
    {
		return CrateBase::AssignIndex( obj);
    } 
  
template <  typename Lambda, typename... Args>
    static auto    OperateOn( Entry *entry, Lambda &lambda,  Args&... args)
    {
        if ( entry->Type() ==  Sz)
            return lambda( static_cast< Elem *>( entry), args...); 
        return CrateBase::OperateOn( entry, lambda, args...);
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
    uint8_t AssignIndex( X *obj)
    { 
        return obj->m_Type = Sz;
    }

template <  typename Lambda, typename... Args>
    static auto    OperateOn( Entry *entry, Lambda &lambda,  Args&... args)
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
	Cv_CrateId		Store( Object *x)
    {
        uint8_t		type = Crate::AssignIndex( x);        
        uint32_t    k = m_Repos->Size();
        m_Repos->SetAt( k, x);
        return *x;
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
            if ( !accum.Accumulate( Crate::OperateOn( si, lambda, args...)))
                return accum;
        }
        return accum;
    }

};  

//_____________________________________________________________________________________________________________________________

template < typename Crate>
struct   Cv_Constructor 
{   
    typedef typename Crate::Entry   Entry; 

    Cv_CrateRepos< Crate>           *m_Crate;
    std::map< void *, Entry *>      m_CnstrMap;

    Cv_Constructor( Cv_CrateRepos< Crate>  *crate) 
        : m_Crate( crate)
    {}

template < typename Node>    
    Entry     *FetchSynTree( Node *node)
    {
        typedef typename Node::SynElem      SynItem;

        auto        res  = m_CnstrMap.emplace( node, ( Entry *) NULL); 
        if ( !res.second)
            return static_cast< SynItem *>( res.first->second);  
        auto        synItem = node->Setup( this);
        res.first->second = synItem;
        return synItem;
    }     
};

//_____________________________________________________________________________________________________________________________

