// cv_crate.h _________________________________________________________________________________________________________________
#pragma once
  
#include    "cove/barn/cv_cexpr.h"
#include    "cove/flux/cv_dotstream.h" 

template < uint32_t>
struct Cv_Typeitem 
{};

//_____________________________________________________________________________________________________________________________

template < typename Crate>
struct	Cv_Var 
{  
	typedef typename Crate::TypeStor	TypeStor;
    typedef typename Crate::Entry		Entry;   

	Entry			    *m_Entry;
	TypeStor			m_Type; 

    Cv_Var( void)
        : m_Entry( NULL), m_Type( 0)
    {}
 
    Cv_Var( Entry *entry, TypeStor typeStor)
		: m_Entry( entry), m_Type( typeStor)
	{}  

    TypeStor        GetType( void) const { return m_Type; } 
	Entry			*GetEntry( void) const { return m_Entry; }  

template < typename Element>    
    Element         *Elem( void) { return m_Type == Crate::template TypeOf< Element>() ? static_cast<Element *>( m_Entry) : NULL; }

    void            Delete( void) { SELF( [] ( auto obj) { delete obj; }); }
   
    operator        bool( void) const { return !!m_Entry; }
    auto            operator->( void) { return m_Entry; }

template < typename Lambda, typename... Args>
    constexpr auto    operator()(  Lambda &&lambda,  Args &&... args)  const   {
		return Crate::Operate(  m_Entry, m_Type, lambda, args...); }  

    friend	bool    operator<( const Cv_Var &id1, const Cv_Var &id2)  { 
        return ( id1.m_Type != id2.m_Type) ? ( id1.m_Type < id2.m_Type) : ( id1.m_Entry < id2.m_Entry) ;  } 


template < typename Lambda, typename... Args>
    constexpr auto    Enact(  Lambda &&lambda,  Args &&... args)  const   
    {
        switch ( m_Type)
        {   
            case 1 :  return EnactCrate( Cv_Typeitem< 1>(), Crate(), m_Entry, lambda, args...); 
            case 2 :  return EnactCrate( Cv_Typeitem< 2>(), Crate(), m_Entry, lambda, args...); 
            case 3 :  return EnactCrate( Cv_Typeitem< 3>(), Crate(), m_Entry, lambda, args...); 
            case 4 :  return EnactCrate( Cv_Typeitem< 4>(), Crate(), m_Entry, lambda, args...); 
            case 5 :  return EnactCrate( Cv_Typeitem< 5>(), Crate(), m_Entry, lambda, args...); 
            case 6 :  return EnactCrate( Cv_Typeitem< 6>(), Crate(), m_Entry, lambda, args...); 
            case 7 :  return EnactCrate( Cv_Typeitem< 7>(), Crate(), m_Entry, lambda, args...); 
            case 8 :  return EnactCrate( Cv_Typeitem< 8>(), Crate(), m_Entry, lambda, args...); 
            case 9 :  return EnactCrate( Cv_Typeitem< 9>(), Crate(), m_Entry, lambda, args...); 
//            default : return EnactCrate( Cv_Typeitem< 0>(), Crate(), m_Entry, lambda, args...); 
        }
    }  

};

//_____________________________________________________________________________________________________________________________

template < typename T, typename... Rest>
struct Cv_Crate : public Cv_Crate< Rest...>
{   
    typedef Cv_Crate< T, Rest...>           Crate;
    typedef Cv_Crate< Rest...>              CrateBase;
    typedef T                               Elem;
    typedef typename CrateBase::Entry       Entry; 
	typedef Cv_Var< Crate>		            Var; 
	typedef typename CrateBase::TypeStor    TypeStor;  
     
    enum { 
        Sz = CrateBase::Sz +1         
    };
    
    
    Cv_Crate( void) 
    { 
    }     

template <typename X, typename std::enable_if< std ::is_base_of< Elem, X>::value, void>::type * = nullptr>
    static constexpr TypeStor TypeOf( X *obj)
    {
        return Sz;
    } 

template < typename X, typename std::enable_if< !std ::is_base_of< Elem, X>::value, void>::type * = nullptr>
    static constexpr TypeStor TypeOf( X *obj)
    {
		return CrateBase::TypeOf( obj);
    } 
  
template <  typename Entity, typename std::enable_if< std ::is_same< Elem, Entity>::value, void>::type * = nullptr>
	static constexpr  TypeStor TypeOf( void)  
	{
		return Sz;
	}

template <  typename Entity, typename std::enable_if< !std ::is_same< Elem, Entity>::value, void>::type * = nullptr>
	static constexpr  TypeStor TypeOf( void)  
	{
		return CrateBase::template TypeOf<Entity> ();
	}

template <  typename Lambda, typename... Args>
    static constexpr auto    Operate( Entry *entry, const TypeStor typeStor, Lambda &&lambda,  Args &&... args)  
    {
        switch ( typeStor)
        {
            case Sz : return lambda( static_cast< Elem *>( entry), args...); 
            default : return CrateBase::Operate( entry, typeStor, lambda, args...);
        }
    } 

template <  typename Lambda, typename... Args>
    friend constexpr auto    EnactCrate( const Cv_Typeitem< Sz> &typeItem, const Crate &crate, Entry *entry, Lambda &&lambda,  Args &&... args)  
    {
        return lambda( static_cast< Elem *>( entry), args...); 
    }
}; 

//_____________________________________________________________________________________________________________________________

template<typename T, typename=void>
struct Cv_CrateT
{  
    enum {
        Sz = 1,
    }; 

	typedef Cv_CrateT< T,void>			    Crate;
    typedef T								Entry;
    typedef T								Elem;
	typedef typename Entry::TypeStor		TypeStor;  
    typedef  Cv_Var< Crate>			        Var; 

template < typename X = void>    
    static constexpr TypeStor	TypeOf( X *obj)
    { 
		return Sz;
    }

template <  typename Entity, typename std::enable_if< std ::is_same< Elem, Entity>::value, void>::type * = nullptr>
	static constexpr  TypeStor TypeOf( void)  
	{
		return Sz;
	}	 

template <  typename Entity, typename std::enable_if< !std ::is_same< Elem, Entity>::value, void>::type * = nullptr>
	static constexpr  TypeStor TypeOf( void)  
	{
		return 0;
	}	


template <  typename Lambda, typename... Args>
    static constexpr auto       Operate( Entry *entry, const TypeStor typeStor, Lambda &&lambda,  Args &&... args)  
    {
        return lambda( static_cast< Elem *>( entry), args...); 
    }

template <  typename Lambda, typename... Args>
    friend constexpr auto    EnactCrate( const Cv_Typeitem< Sz> &typeItem, const Crate &crate, Entry *entry, Lambda &&lambda,  Args &&... args)  
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
