//_______________________________________________ cv_crate.h _______________________________________________________________
#pragma once
 
#include    "cove/silo/cv_repos.h"
#include    "cove/barn/cv_cexpr.h" 

//_____________________________________________________________________________________________________________________________

class  Cv_CrateId 
{
public:	
	typedef uint32_t	IPtrStor;	
	typedef uint32_t	IndexStor;	
	typedef uint32_t	TypeStor;	 
	enum {
		SzTypeBits	= 8,
		SzIPtrBits	= sizeof( IPtrStor) * 8 -SzTypeBits,
		MaskIPtr	= Cv_CExpr::LowMask( SzIPtrBits)
	};

	IPtrStor			m_IPtr; 
	
public:
	Cv_CrateId( IndexStor id, TypeStor type)
		:  m_IPtr( ( MaskIPtr & id) | ( type << SzIPtrBits))
	{} 


	IndexStor		GetId( void) const { return IndexStor( MaskIPtr & m_IPtr); } 
	void            SetId( IndexStor k) { m_IPtr = ( MaskIPtr & k) | ( m_IPtr & ~MaskIPtr); }

	TypeStor        GetType( void) const { return TypeStor(  m_IPtr >> SzIPtrBits ); }
	TypeStor		SetType( TypeStor k) { return  m_IPtr = (( MaskIPtr & m_IPtr) | ( k << SzIPtrBits)); }
};

//_____________________________________________________________________________________________________________________________

class  Cv_CrateEntry  : public Cv_CrateId
{ 
public:
    Cv_CrateEntry( IndexStor id = IndexStor( -1))
        :  Cv_CrateId( id, 0)
    {}

	const char      *GetName( void) const { return ""; } 

template <  typename Crate,  typename Lambda, typename... Args>
	auto    Operate( TypeStor type, Lambda lambda,  Args&... args)  
	{
		return Crate::Operate( type, static_cast< typename Crate::Entry *>( this), lambda, args...);
	} 

template <  typename Crate,  typename Lambda, typename... Args>
    auto    Operate(  Lambda lambda,  Args&... args)  
    {
        return Crate::Operate( GetType(), static_cast< typename Crate::Entry *>( this), lambda, args...);
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
    typedef Cv_Crate< T, Rest...>			Crate;
    typedef Cv_Crate< Rest...>				CrateBase;
    typedef T								Elem;
    typedef typename CrateBase::Entry		Entry; 
	typedef typename CrateBase::TypeStor    TypeStor; 

    enum {
        Sz = CrateBase::Sz +1,
    };
      
    Cv_Crate( void) 
    {}

template <typename X, typename std::enable_if< std ::is_base_of< T, X>::value, void>::type * = nullptr>
	TypeStor AssignIndex( X *obj)
    {
        return obj->SetType( TypeStor( Sz));
    } 

template < typename X, typename std::enable_if< !std ::is_base_of< T, X>::value, void>::type * = nullptr>
	TypeStor AssignIndex( X *obj)
    {
		return CrateBase::AssignIndex( obj);
    } 
  
template <  typename Lambda, typename... Args>
    static auto    Operate( TypeStor type, Entry *entry, Lambda &lambda,  Args&... args)  
    {
        if ( type ==  Sz)
            return lambda( static_cast< Elem *>( entry), args...); 
        return CrateBase::Operate( type, entry, lambda, args...);
    }
}; 

//_____________________________________________________________________________________________________________________________

template<typename T, typename=void>
struct Cv_CrateT
{ 
    enum {
        Sz = 1, 
    }; 

    typedef  T							Entry;
    typedef T							Elem;
	typedef typename Entry::TypeStor    TypeStor;

template < typename X = void>    
	TypeStor AssignIndex( X *obj)
    { 
		return obj->m_Type =  Sz;
    }

template <  typename Lambda, typename... Args>
    static auto    Operate( TypeStor type, Entry *entry, Lambda &lambda,  Args&... args)  
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
	typedef typename T::IndexStor	IndexStor;	 
	typedef typename T::TypeStor	TypeStor;
	 
}; 

//_____________________________________________________________________________________________________________________________

template < typename Crate>
class Cv_CrateRepos  : public Crate
{ 
public: 
    typedef typename Crate::Entry                           Entry; 
	typedef typename Entry::TypeStor						TypeStor; 
	typedef typename Entry::IndexStor						IndexStor;  

protected:
	std::vector< Entry *>				m_Elems;
	std::vector< TypeStor>				m_Types;
   
public: 

    Cv_CrateRepos( void) 
    {
		m_Elems.push_back( NULL); 
		m_Types.push_back( 0); 
	}

    void Clear( void)
    {
        OperateAll( []( auto x) { delete x; return true; }); 
        m_Elems.clear();
		m_Types.clear();
    }

	uint32_t    Size( void) const { return uint32_t( m_Elems.size()); }

template<  class Object>
    void    Store( Object *x)
    {
		TypeStor	typeVal = Crate::AssignIndex( x);   
		x->SetId( IndexStor( m_Elems.size()));
		m_Elems.push_back( x); 
		m_Types.push_back( typeVal); 
        return;
    }
 
template < typename Lambda, typename... Args>
    auto    OperateAll(  Lambda lambda,  Args&... args)  
    {   
        typedef Cv_CrateLambdaAccum< decltype( lambda(  static_cast<Entry *>( nullptr), args...))>     Accum;
        Accum                                               accum;
        for ( uint32_t i = 0; i < Size(); ++i)
        {
            Entry     *si = m_Elems[ i]; 
            if ( !si)
                continue; 
            if ( !accum.Accumulate( Crate::Operate( m_Types[ i], si, lambda, args...)))
                return accum;
        }
        return accum;
    } 

template < typename Lambda, typename... Args>
	auto    Operate( IndexStor iptr, Lambda lambda,  Args&... args)  
	{   
		typedef Cv_CrateLambdaAccum< decltype( lambda(  static_cast<Entry *>( nullptr), args...))>     Accum;
		Accum                                               accum; 
		Entry     *si = m_Elems[ iptr]; 
		if ( !si)
			return accum; 
		accum.Accumulate( Crate::Operate( m_Types[ iptr], si, lambda, args...));
		return accum;
	} 
};  

//_____________________________________________________________________________________________________________________________

template < typename Crate>
struct   Cv_CrateConstructor 
{  
typedef typename Crate::Entry                           Entry; 

	Cv_CrateRepos< Crate>			*m_Crate;
	std::map< void *, Entry *>      m_CnstrMap;

	Cv_CrateConstructor( Cv_CrateRepos< Crate>  *crate) 
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

