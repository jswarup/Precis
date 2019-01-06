//_______________________________________________ cv_crate.h _______________________________________________________________
#pragma once
 
#include    "cove/silo/cv_repos.h"
#include    "cove/barn/cv_cexpr.h"
#include    "cove/silo/cv_dotstream.h"
  
//_____________________________________________________________________________________________________________________________

template < typename Crate>
struct	Cv_Var 
{
	typedef typename Crate::TypeStor	TypeStor;
	typedef typename Crate::Entry		Entry; 

	Entry				*m_Entry;
	TypeStor			m_Type; 

	Cv_Var( Entry *entry, TypeStor typeStor)
		: m_Entry( entry), m_Type( typeStor)
	{} 
		 
	TypeStor        GetType( void) const { return m_Type; }
	Entry			*GetEntry( void) const { return m_Entry; } 

template < typename Lambda, typename... Args>
	auto    operator()( Lambda &lambda,  Args&... args)  
	{
		return Crate::Operate( static_cast< Entry *>( m_Entry), m_Type, lambda, args...);
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
	typedef typename Cv_Var< Crate>		Var; 
	typedef typename Entry::TypeStor    TypeStor; 
    
    enum {
        Sz = CrateBase::Sz +1,
    };
     
    Cv_Crate( void) 
    {
        
    }     

template <typename X, typename std::enable_if< std ::is_base_of< T, X>::value, void>::type * = nullptr>
	TypeStor AssignIndex( X *obj)
    {
        return obj->SetType(  Sz);
    } 

template < typename X, typename std::enable_if< !std ::is_base_of< T, X>::value, void>::type * = nullptr>
	TypeStor AssignIndex( X *obj)
    {
		return CrateBase::AssignIndex( obj);
    } 
  
template <  typename Lambda, typename... Args>
    static auto    Operate( Entry *entry, TypeStor typeStor, Lambda &lambda,  Args&... args)  
    {
        switch ( typeStor)
		{
			case Sz : return lambda( static_cast< Elem *>( entry), args...); 
			default : return CrateBase::Operate( entry, typeStor, lambda, args...);
		}
    } 
	 
template <  typename Entity, typename std::enable_if< std ::is_same< T, Entity>::value, void>::type * = nullptr>
	static constexpr  TypeStor TypeOf( void)  
	{
		return Sz;
	}

template <  typename Entity, typename std::enable_if< !std ::is_same< T, Entity>::value, void>::type * = nullptr>
	static constexpr  TypeStor TypeOf( void)  
	{
		return CrateBase::TypeOf<Entity>();
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
    typedef  T								Entry;
    typedef T								Elem;
	typedef typename Entry::TypeStor		TypeStor; 
	typedef typename Cv_Var< Crate>			Var; 

template < typename X = void>    
	TypeStor	AssignIndex( X *obj)
    { 
		return obj->m_Type =  Sz;
    }

template <  typename Lambda, typename... Args>
    static auto    Operate( Entry *entry, TypeStor typeStor, Lambda &lambda,  Args&... args)  
    {
		 return lambda( static_cast< Elem *>( entry), args...); 
    }

template <  typename Entity, typename std::enable_if< std ::is_same< T, Entity>::value, void>::type * = nullptr>
	static constexpr  TypeStor TypeOf( void)  
	{
		return Sz;
	}	 

template <  typename Entity, typename std::enable_if< !std ::is_same< T, Entity>::value, void>::type * = nullptr>
	static constexpr  TypeStor TypeOf( void)  
	{
		return 0;
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
	Cv_CrateId( void)
		: m_IPtr( 0)
	{}
	Cv_CrateId( IndexStor id, TypeStor type)
		:  m_IPtr( ( MaskIPtr & id) | ( type << SzIPtrBits))
	{} 


	IndexStor		GetId( void) const { return IndexStor( MaskIPtr & m_IPtr); } 
	void            SetId( IndexStor k) { m_IPtr = ( MaskIPtr & k) | ( m_IPtr & ~MaskIPtr); }

	TypeStor        GetType( void) const { return TypeStor(  m_IPtr >> SzIPtrBits ); }
	TypeStor		SetType( TypeStor k) {   m_IPtr = (( MaskIPtr & m_IPtr) | ( k << SzIPtrBits)); return k; }

	friend	Cv_DotStream    &operator<<( Cv_DotStream  &dotStrm, const Cv_CrateId &x)  
	{ 
		dotStrm.OStream() << "Id" << x.m_IPtr;
		return dotStrm;
	}
};

//_____________________________________________________________________________________________________________________________

class  Cv_CrateEntry : public Cv_CrateId
{
public:   

public:
	Cv_CrateEntry( uint32_t id = CV_UINT32_MAX)
		:  Cv_CrateId( id, 0)
	{} 

	const char		*GetName( void) const { return "Entry"; }


	friend	Cv_DotStream    &operator<<( Cv_DotStream  &dotStrm, const Cv_CrateEntry *x)  
	{ 
		dotStrm.OStream() << x->GetName() << '_' <<  x->GetId();
		return dotStrm;
	} 
};  

//_____________________________________________________________________________________________________________________________

template < typename Crate>
class Cv_CrateRepos  : public Crate
{ 
public: 
    typedef typename Crate::Entry           Entry; 
	typedef typename Entry::TypeStor		TypeStor; 
	typedef typename Entry::IndexStor		IndexStor;
	typedef typename Crate::Var				Var; 

protected:
	std::vector< Entry *>					m_Elems;
	std::vector< TypeStor>					m_Types;
   
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

	Var			ToVar( Cv_CrateId id) { return Var( m_Elems[ id.GetId()], id.GetType()); }

	Var			Get( uint32_t k) { return Var( m_Elems[ k], m_Types[ k]); }

	void        Destroy( uint32_t k)
	{ 
		Entry       *&elem = m_Elems[ k];
		TypeStor	&type = m_Types[ k];
		Crate::Operate( elem, type, []( auto x) { delete x; });
		type = 0; 
		elem = NULL; 
		return;
	}

template<  class Object>
	Cv_CrateId    Store( Object *x)
    {
		TypeStor	typeVal = Crate::AssignIndex( x); 

		IndexStor	ind = IndexStor( m_Elems.size());
		x->SetId( ind);
		m_Elems.push_back( x); 
		m_Types.push_back( typeVal); 
        return Cv_CrateId( ind, typeVal);
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
            if ( !accum.Accumulate( Crate::Operate( si, m_Types[ i], lambda, args...)))
                return accum;
        }
        return accum;
    }

};

//_____________________________________________________________________________________________________________________________

template < typename Crate>
struct   Cv_CrateConstructor 
{  
	typedef Crate 						Crate; 	
	typedef typename Crate::Entry		Entry; 
	typedef typename Crate::Var			Var; 

	Cv_CrateRepos< Crate>				*m_Crate;
	std::map< void *, Cv_CrateId>		m_CnstrMap;

	Cv_CrateConstructor( Cv_CrateRepos< Crate>  *crate) 
		: m_Crate( crate)
	{}

	auto			Repos( void) { return m_Crate; }

template<  class Object>
	Cv_CrateId		Store( Object *x) { return m_Crate->Store( x); }

template < typename Node>    
	Cv_CrateId	FetchElemId( Node *node)
	{ 

		auto        res  = m_CnstrMap.emplace( node, Cv_CrateId()); 
		if ( !res.second)
			return  res.first->second;  
		Cv_CrateId	item = node->FetchElemId( this); 
		res.first->second = item;
		return item;
	}     
};

//_____________________________________________________________________________________________________________________________
