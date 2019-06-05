// cv_craterepos.h ____________________________________________________________________________________________________________
#pragma once

#include    "cove/silo/cv_repos.h"
#include    "cove/barn/cv_cexpr.h"
#include    "cove/barn/cv_aid.h"
#include    "cove/silo/cv_crate.h" 

//_____________________________________________________________________________________________________________________________

class  Cv_CrateId 
{
public:	 
    typedef uint64_t	    IPtrStor;	
    typedef uint64_t	    IndexStor;	
    typedef uint8_t	        TypeStor;	 

    enum {
        SzTypeBits	= 8,
        SzIPtrBits	= ( sizeof( IPtrStor) * 8) -SzTypeBits,
    };
    
    static constexpr uint64_t IPtrMask( void) {	return Cv_CExpr::LowMask( SzIPtrBits); }
    
    IPtrStor			m_IPtr; 

public:
    Cv_CrateId( void)
        : m_IPtr( 0)
    {}

    Cv_CrateId( IndexStor id, TypeStor type)
        :  m_IPtr( ( IPtrMask() & id) | ( IPtrStor( type) << SzIPtrBits))
    {} 

    Cv_CrateId( const Cv_CrateId &id)
        :  m_IPtr( id.m_IPtr)
    {}

    Cv_CrateId( Cv_CrateId &&id)
        :  m_IPtr( id.m_IPtr)
    {}

    bool            IsValid( void) const { return !!m_IPtr; }

    uint32_t 		GetId( void) const { return uint32_t( IndexStor( IPtrMask() & m_IPtr)); } 
    void            SetId( IndexStor k) { m_IPtr = ( IPtrMask() & k) | ( m_IPtr & ~IPtrMask()); }

    TypeStor        GetType( void) const { return TypeStor( m_IPtr >> SzIPtrBits ); }
    TypeStor		SetType( TypeStor k) {   m_IPtr = (( IPtrMask() & m_IPtr) | ( IPtrStor( k) << SzIPtrBits)); return k; }
    char            GetTypeChar( void) const
    {
        TypeStor    type = GetType();
        if ( type < 26)
            return char( 'A' +type);
        if ( type < 52)
            return char( 'a' +type -26);
        CV_ERROR_ASSERT( false)
        return '_';
    }

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
        typedef void    Copiable;

        Id( void) {}

        Id( const Cv_CrateId &id) 
            :  Cv_CrateId( id) 
        {}

        Id( IndexStor id, TypeStor type) 
            :  Cv_CrateId( id, type) 
        {} 
    };
public:
    typedef void    Repos;

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
    struct Cask : public Cv_SerializeUtils 
    {     
        typedef  Cv_Cask< Entry *>                  SubCask;
        typedef  typename SubCask::ContentType      SubContent; 

        struct  ContentType
        {
            uint32_t        m_Size; 
            uint32_t        m_TypeOffset;

            typedef void Copiable;  
            
            Cv_CArr< SubContent>     Elems( void)
            { 
                return Cv_CArr< SubContent>( ( SubContent *) ( cv_pcast< uint8_t>( this) + sizeof( ContentType)), m_Size);
            }

            Cv_CArr< TypeStor>       Types( void)
            { 
                return Cv_CArr< TypeStor>( ( TypeStor *) ( cv_pcast< uint8_t>( this) + m_TypeOffset), m_Size);
            }
        };  

        static uint32_t         ContentSize( const Cv_CrateRepos &obj) { return   sizeof( ContentType) + obj.Size() * sizeof( SubContent) +sizeof( TypeStor) * obj.Size(); }

        static uint32_t         Spread( ContentType *obj) 
        {
            uint32_t                sz = ContentSize( *obj); 
            Cv_CArr< SubContent>    subArr = obj->Value(); 
            for ( uint32_t i = 0; i < subArr.Size(); ++i)
                sz += SubCask::Spread( &subArr[ i]);
            return sz;
        }

        static ContentType         Encase( Cv_Spritz *spritz, const Cv_CrateRepos &repos)
        {
            uint64_t        off = spritz->Offset(); 
            spritz->EnsureSize( ContentSize( repos));
            spritz->SetOffsetAtEnd(); 
            ContentType    fileObj;
            fileObj.m_Size = repos.Size();
            spritz->SetOffset( off +sizeof( ContentType)); 
            for ( uint32_t i = 0; i < repos.Size(); ++i)
            {
                Entry       *elem = repos.m_Elems[ i];
                uint32_t    type = repos.m_Types[ i];  
                Var( elem, repos.m_Types[ i])( [ spritz, type]( auto x) { 
                    Cv_Aid::Save( spritz, x);    
                    return true; }
                );
            }  
            spritz->SetOffsetAtEnd();
            fileObj.m_TypeOffset = uint32_t( spritz->Offset() -off);
            bool    res = spritz->Write( &repos.m_Types[ 0], sizeof( TypeStor) * fileObj.m_Size); 
            spritz->SetOffset( off);
            return fileObj;
        } 

        static ContentType     *Bloom( uint8_t *arr)
        {
            return ( ContentType *) arr;
        } 
    };  

    Cv_CrateRepos( void) 
    {
        m_Elems.push_back( NULL); 
        m_Types.push_back( 0); 
    }
    
    ~Cv_CrateRepos( void)  { Clear(); }

template < typename X>
    auto        Delete( X *obj, int k) ->   decltype( X().Delete( nullptr) )
        { obj->Delete( static_cast< typename X::Repos *>( this));   }

template<typename X> 
    void    Delete( X *obj, ...)  
        { delete obj; }

    auto        Deleter( void)
    {
        return [ this]( auto x) { 
            Delete( x, 0); 
            return true; };
    }
    void        Clear( void)
    {
        OperateAll( Deleter()); 
        m_Elems.clear();
        m_Types.clear();
    }

    uint32_t    Size( void) const { return uint32_t( m_Elems.size()); }

template < typename Element>
    static Id   ToId( Element *e) { return Id( e->GetId(), Crate::template TypeOf< Element>()); }

    static Id   ToId( Var v) { return Id( v.GetEntry()->GetId(), v.GetType()); }

    Var			ToVar( Id id) { return Var( m_Elems[ id.GetId()], id.GetType()); }

    Var			Get( uint32_t k) { return Var( m_Elems[ k], m_Types[ k]); }

    Id			GetId( uint32_t k) { return Id( k, m_Types[ k]); }

    void        Destroy( uint32_t k)
    { 
        Entry       *&elem = m_Elems[ k];
        TypeStor	&type = m_Types[ k];
        Crate::Operate( elem, type, Deleter());
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
    auto    OperateAll(  Lambda &&lambda,  Args&&... args)  
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

    bool    DumpStats( std::ostream &ostr)
    {
        ostr << typeid(Entry).name() <<  "Count: " << m_Elems.size() << "\n"; 
        return true;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename CrateT>
struct      Cv_CrateConstructor 
{  
    typedef CrateT 						                Crate; 	
    typedef typename Crate::Entry		                Entry; 
    typedef typename Crate::Var			                Var; 
    typedef typename Cv_CrateRepos< Crate>::Id			Id; 

    Cv_CrateRepos< Crate>				*m_Crate;
    std::map< void *, Id>		        m_CnstrMap;

    Cv_CrateConstructor( Cv_CrateRepos< Crate>  *crate) 
        : m_Crate( crate)
    {}

    auto    Repos( void) { return m_Crate; }

template<  class Object>
    Id	    Store( Object *x) { return m_Crate->Store( x); }

template < typename Node>    
    Id	    FetchElemId( Node *node)
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

template < typename Crate, typename=void>
struct Cv_CratePile : public Cv_CratePile< typename Crate::CrateBase>
{
    typedef typename Crate::CrateBase       CrateBase;

    typedef Cv_CratePile< CrateBase>        Base;

    typedef typename Crate::Entry           Entry;
    typedef typename Crate::Elem            Elem;
    typedef typename Base::Id               Id;
    typedef typename Crate::Var	            Var; 
    typedef typename Entry::TypeStor		TypeStor; 
    typedef typename Entry::IndexStor		IndexStor; 

    std::vector< Elem>                      m_Elems;
    
    
    struct Cask : public Cv_MemberCask< std::vector< Elem>>, public Base::Cask
    { 
        typedef typename Cv_CratePile< CrateBase>::Cask     BaseCask;
        typedef Cv_MemberCask< std::vector< Elem>>          ItemCask;
        typedef typename ItemCask::ContentType              ItemContent;
        typedef typename BaseCask::ContentType              BaseContent;

        struct  ContentType : public BaseContent
        {
            ItemContent    m_Value;

            ContentType( const BaseContent &t2, const ItemContent &t1)
                : BaseContent( t2), m_Value( t1)
            {} 
        };  

        static uint32_t         ContentSize( const ContentType &obj) { return   sizeof( ContentType); }

        static uint32_t         Spread( ContentType *obj) 
        {
            return BaseCask().Spread( obj) +ItemCask().Spread( &obj->m_Value);
        }

        static ContentType      Encase( Cv_Spritz *spritz, const Cv_CratePile &obj)
        {   
            ContentType     *fileObj = NULL;
            spritz->EnsureSize( ContentSize( *fileObj));   
            uint64_t    off = spritz->Offset();
            auto        bc = BaseCask::Encase( spritz,  obj); 
            spritz->SetOffset( off + uint64_t( &reinterpret_cast< ContentType *>( 0x8)->m_Value) - 0x8);
            auto        ic = ItemCask::Encase( spritz, obj.m_Elems);
            spritz->SetOffset( off);
            return ContentType( bc, ic);
        }

        static ContentType      *Bloom( uint8_t *arr)
        {
            return ( ContentType *) arr;
        }
    }; 
     

    struct Blossom
    {
        typedef typename Cask::ContentType  ContentType;
        typedef typename Base::Blossom      BaseBlossom;
        
        ContentType    *m_Root;

        Blossom( ContentType  *arr)
            : m_Root(  arr)
        {}

        Var         ToVar( const Id &id)  
        {  
            uint32_t    k = Crate::Sz; 
            switch ( id.GetType())
            {
                case  Crate::Sz:    
                {
                    auto arr = m_Root->m_Value.m_Value.Value();
                    return Var( &arr[ id.GetId()], id.GetType()); 
                }   
                default :  auto   var = BaseBlossom( m_Root).ToVar( id); return Var( var.GetEntry(), var.GetType());
            }
            return Var();
        }
    };

    Id    Push( const Elem &elm) 
    { 
        m_Elems.push_back( elm); 
        Elem    *insrt =  &m_Elems.back();
        insrt->SetId( IndexStor( m_Elems.size() -1));
        insrt->SetType( TypeStor( Crate::Sz));
        return *insrt; 
    } 

    Id          Store( const Entry &entry) 
    { 
        switch ( entry.GetType())
        {
            case  Crate::Sz : return Push( static_cast< const Elem &>( entry));
            default :  return Base::Store( entry);
        }
    }   

    Var     ToVar( const Id &id)  
    {  
        switch ( id.GetType())
        {
            case  Crate::Sz:    return Var(  &m_Elems[ id.GetId()], id.GetType()); 
            default :           auto    var = Base::ToVar( id); return Var( var.GetEntry(), var.GetType());
        }
    }

template < typename Lambda, typename... Args>
    auto    OperateAll( const Lambda &lambda,  Args&&... args)  
    {   
        typedef Cv_CrateLambdaAccum< decltype( lambda(  static_cast<Entry *>( nullptr), args...))>     Accum;
        Accum                                               accum;
        for ( uint32_t i = 0; i < m_Elems.size(); ++i)         
            if ( !accum.Accumulate( lambda( &m_Elems[ i], args...)))  
                return accum; 
        return accum.Accumulate( Base::OperateAll( lambda, args...));
    }

    bool                DumpStats( std::ostream &ostr)
    {
        ostr << typeid(Elem).name() <<  "Count: " << m_Elems.size() << "\n";
        Base::DumpStats( ostr);
        return true;
    }
};

//_____________________________________________________________________________________________________________________________

template<typename Crate>
struct  Cv_CratePile< Crate, typename  Cv_TypeEngage::Same< typename Crate::Elem, typename Crate::Entry>::Note>  
{ 

    typedef typename Crate::Entry       Entry;
    typedef typename Crate::Elem        Elem;
    typedef typename Entry::Id          Id;
    typedef typename Crate::Var         Var;

    std::vector< Elem>                  m_Elems; 


    struct Cask : public Cv_MemberCask< std::vector< Elem>>
    {  
        typedef Cv_MemberCask< std::vector< Elem>>    ItemCask; 
        typedef typename ItemCask::ContentType      ItemContent;

        struct  ContentType 
        {
            ItemContent     m_Value;

            ContentType( const ItemContent &t1)
                : m_Value( t1)
            {} 
        };  

        static uint32_t        Spread( ContentType *obj, const Cv_CArr< uint8_t> &arr) 
        {
            return ItemCask().Spread( &obj->m_Value, arr);
        }

        static ContentType     Encase( Cv_Spritz *spritz, const Cv_CratePile &obj)
        { 
            return ContentType( ItemCask::Encase( spritz, obj.m_Elems));
        }

        static ContentType     *Bloom( uint8_t *arr)
        {
            return ( ContentType *) arr;
        }
    }; 
 

    struct Blossom
    {
        typedef typename Cask::ContentType  ContentType;

        ContentType    *m_Root;

        Blossom( ContentType  *arr)
            : m_Root( arr)
        {}

        Var         ToVar( const Id &id)  
        {  
            auto arr = m_Root->m_Value.m_Value.Value();
            return Var( &arr[ id.GetId()], id.GetType()); 
        }
    };
     
    Id          Push( const Elem &elm) 
    { 
        m_Elems.push_back( elm); 
        Elem    &insrt =  m_Elems.back();
        insrt.SetId( uint32_t( m_Elems.size() -1));
        insrt.SetType( Crate::Sz);
        return insrt;  
    }  

    Id          Store( const Entry &entry) 
    {    
        return Push( static_cast< const Elem &>( entry));
    }   
 
    Var         ToVar( const Id &id)  
    {  
        return Var(  &m_Elems[ id.GetId()], id.GetType()); 
    }

template < typename Lambda, typename... Args>
    auto    OperateAll(  Lambda &&lambda,  Args&&... args)  
    {   
        typedef Cv_CrateLambdaAccum< decltype( lambda(  static_cast<Entry *>( nullptr), args...))>     Accum;
        Accum                                               accum;
        for ( uint32_t i = 0; i < m_Elems.size(); ++i)  
            if ( !accum.Accumulate( lambda( &m_Elems[ i], args...)))        
                return accum; 
        return accum;
    }

    bool                DumpStats( std::ostream &ostr)
    {
        ostr << typeid(Elem).name() <<  "Count: " << m_Elems.size() << "\n"; 
        return true;
    }
};

//_____________________________________________________________________________________________________________________________
