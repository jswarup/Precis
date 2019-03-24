// cv_craterepos.h ____________________________________________________________________________________________________________
#pragma once

#include    "cove/silo/cv_repos.h"
#include    "cove/barn/cv_cexpr.h"
#include    "cove/silo/cv_crate.h" 

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

    Cv_CrateId( const Cv_CrateId &id)
        :  m_IPtr( id.m_IPtr)
    {}

    Cv_CrateId( Cv_CrateId &&id)
        :  m_IPtr( id.m_IPtr)
    {}

    bool            IsValid( void) const { return !!m_IPtr; }

    IndexStor		GetId( void) const { return IndexStor( MaskIPtr & m_IPtr); } 
    void            SetId( IndexStor k) { m_IPtr = ( MaskIPtr & k) | ( m_IPtr & ~MaskIPtr); }

    TypeStor        GetType( void) const { return TypeStor(  m_IPtr >> SzIPtrBits ); }
    TypeStor		SetType( TypeStor k) {   m_IPtr = (( MaskIPtr & m_IPtr) | ( k << SzIPtrBits)); return k; }

    Cv_CrateId      &operator=( const Cv_CrateId &id) { m_IPtr = id.m_IPtr; return SELF; } 

    friend	bool    operator<( const Cv_CrateId &id1, const Cv_CrateId &id2) { return id1.m_IPtr < id2.m_IPtr;  } 
    friend	bool    operator==( const Cv_CrateId &id1, const Cv_CrateId &id2) { return id1.m_IPtr == id2.m_IPtr;  } 
    friend	bool    operator!=( const Cv_CrateId &id1, const Cv_CrateId &id2) { return !( id1.m_IPtr == id2.m_IPtr);  } 

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
    struct Id : public Cv_CrateId 
    { 
        Id( void) {}

        Id( const Cv_CrateId &id) 
            :  Cv_CrateId( id) 
        {}

        Id( IndexStor id, TypeStor type) 
            :  Cv_CrateId( id, type) 
        {} 
    };
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
    typedef typename Entry::Id		        Id;


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

template < typename Element>
    static Id   ToId( Element *e) { return Id( e->GetId(), TypeOf< Element>()); }

    static Id   ToId( Var v) { return Id( v.Entry()->GetId(), v.GetType()); }

    Var			ToVar( Id id) { return Var( m_Elems[ id.GetId()], id.GetType()); }

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
    Id    Store( Object *x)
    {
        TypeStor	typeVal = Crate::TypeOf( x); 
        x->SetType(  typeVal); 
        IndexStor	ind = IndexStor( m_Elems.size());
        x->SetId( ind);
        m_Elems.push_back( x); 
        m_Types.push_back( typeVal); 
        return Id( ind, typeVal);
    }

template<  class Object>
    Id    StoreAt( uint32_t ind, Object *x)
    {
        TypeStor	typeVal = Crate::TypeOf( x);
        x->SetType(  typeVal);   
        x->SetId( ind);
        Id   id;
        if ( m_Elems[ ind])
            id = Id( ind, m_Elems[ ind]->GetType());
        m_Elems[ ind] = x; 
        m_Types[ ind] = typeVal; 
        return id;
    }

    template<  class Object>
    Object  *Construct( void)
    {
        Object  *x = new Object();
        Store( x);
        return x;
    }

    void            Shrivel( uint32_t m)
    { 
        CV_ERROR_ASSERT( m <= Size())
        for ( ; m < Size(); ++m)
            Destroy( m);
        m_Elems.resize( m);
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
            if ( !accum.Accumulate( Crate::Operate( si, m_Types[ i], lambda, args...)))
                return accum;
        }
        return accum;
    }

};

//_____________________________________________________________________________________________________________________________

template < typename CrateT>
struct   Cv_CrateConstructor 
{  
    typedef CrateT 						                Crate; 	
    typedef typename Crate::Entry		                Entry; 
    typedef typename Crate::Var			                Var; 
    typedef typename Cv_CrateRepos< Crate>::Id			Id; 

    Cv_CrateRepos< Crate>				*m_Crate;
    std::map< void *, Id>		m_CnstrMap;

    Cv_CrateConstructor( Cv_CrateRepos< Crate>  *crate) 
        : m_Crate( crate)
    {}

    auto			Repos( void) { return m_Crate; }

    template<  class Object>
    Id		        Store( Object *x) { return m_Crate->Store( x); }

    template < typename Node>    
    Id	            FetchElemId( Node *node)
    { 

        auto            res  = m_CnstrMap.emplace( node, Id()); 
        if ( !res.second)
            return  res.first->second;  
        Id          item = node->FetchElemId( this); 
        res.first->second = item;
        return item;
    }     
};

//_____________________________________________________________________________________________________________________________
