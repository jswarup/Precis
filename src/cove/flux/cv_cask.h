// cv_cask.h ______________________________________________________________________________________________________________
#pragma once
 
#include	"cove/flux/cv_spritz.h"
#include	"cove/silo/cv_array.h"
#include    "cove/silo/cv_crate.h" 

//_____________________________________________________________________________________________________________________________

template < typename T, typename = void>
struct Cv_Cask
{ 
    typedef T       Type; 
};

//_____________________________________________________________________________________________________________________________

class Cv_SerializeUtils
{ 
    Cv_SerializeUtils( void)
    {}

public:  
template < typename Spritz, typename T>
    static void   SaveContent( Spritz *spritz, const T &t) 
    {
        bool    res = spritz->Write( &t, sizeof( t));  
    }
template < typename Spritz, typename T>
    static void  Save( Spritz *spritz, const T &t) 
    {
        auto    cnt = Cv_Cask< T>::Encase( spritz, t);
        Cv_Cask< T>::SaveContent( spritz, cnt);  
    } 
 
template < typename Spritz, typename T>
    static void  Save( Spritz *spritz, T *t) 
    {
        auto    cnt = Cv_Cask< T*>::Encase( spritz, t);
        Cv_Cask< T *>::SaveContent( spritz, cnt);  
    }  
/*
template < typename Spritz, typename Crate>
    static void  Save( Spritz *spritz, Cv_Var< Crate> var) 
    {
        var( [ spritz]( auto *x) { 
            typedef std::remove_pointer<decltype(x)>::type  DynType;
//            Cv_DynCask< DynType>    
            auto    cnt = Cv_CaskVar< T*>::Encase( spritz, x);
            bool    res = spritz->Write( &cnt, Cv_Cask< T*>::ContentSize( cnt));  
            return true; });
    }
*/
}; 

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Cask< T, typename Cv_TrivialCopy< T>::Note> : public Cv_SerializeUtils 
{      
    typedef T           Type;
    typedef T           ContentType;   

    static uint32_t         ContentSize( const T &obj) { return  sizeof( ContentType); }

    static uint32_t         Spread( ContentType *obj) { return sizeof( *obj); }
    
    static ContentType      Encase( Cv_Spritz *spritz, const T &obj)
    { 
        return obj;
    }
    
    static ContentType      *Bloom( uint8_t *arr)
    {
        return ( ContentType *) arr;
    } 
}; 


//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Cask< Cv_Seq< T> > : public Cv_SerializeUtils 
{
    typedef T                               Type; 
    typedef  Cv_Cask< Type>                 SubCask;
    typedef typename SubCask::ContentType   SubContent;

    struct  ContentType
    {
        uint32_t    m_Offset;
        uint32_t    m_Size;

        typedef void Copiable;  
 
        Cv_Seq< SubContent>     Value( void)
        { 
            return Cv_Seq< SubContent>( ( SubContent *) ( cv_pcast< uint8_t>( this) + m_Offset), m_Size);
        }
    }; 
 
    static uint32_t         Spread( ContentType *obj) 
    {
        uint32_t                sz = sizeof( *obj); 
        Cv_Seq< SubContent>    subArr = obj->Value();
        uint32_t                off = obj->m_Offset;
        for ( uint32_t i = 0; i < subArr.Size(); ++i, off += sizeof( SubContent))
            sz += SubCask::Spread( &subArr[ i]);
        return sz;
    }
    static ContentType         Encase( Cv_Spritz *spritz, const Cv_Seq< Type> &obj)
    {
        spritz->EnsureSize( sizeof( ContentType));
        uint64_t        off = spritz->Offset();
        spritz->SetOffsetAtEnd();
        ContentType    fileObj;
        fileObj.m_Offset = uint32_t( spritz->Offset() -off);
        fileObj.m_Size = obj.Size();
        for ( auto it = obj.Begin(); it != obj.End(); ++it)
            Save( spritz, *it);    
        spritz->SetOffset( off); 
        return fileObj;
    } 
 
    static ContentType     *Bloom( uint8_t *arr)
    {
        return ( ContentType *) arr;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Cask< std::vector< T> > : public Cv_Cask< Cv_Seq< T> > 
{  
    typedef Cv_Cask< Cv_Seq< T>>       BaseCask;
 

    static auto             Encase( Cv_Spritz *spritz, const std::vector< T> &obj)
    {
        return BaseCask::Encase( spritz, Cv_Seq< T>( obj.size() ? ( T *) &obj[ 0] : NULL, uint32_t( obj.size())));
    }
}; 

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Cask< T *> : public Cv_SerializeUtils 
{
    typedef T                               Type; 
    typedef  Cv_Cask< Type>                 SubCask;
    typedef typename SubCask::ContentType   SubContent;

    struct  ContentType
    {
        uint32_t            m_Offset; 

        typedef void        Copiable; 
 
        ContentType( void)
            : m_Offset( 0)
        {}

        SubContent  *Value( void) {   return m_Offset ? ( SubContent *) ( cv_pcast< uint8_t>( this) + m_Offset) : NULL; }
    };  

    static uint32_t         Spread( ContentType *obj) 
    {
        uint32_t                sz = sizeof( *obj); 
        sz += SubCask::Spread( obj->Value());
        return sz;
    }
    static ContentType      Encase( Cv_Spritz *spritz, const Type *obj)
    {
        uint64_t        off = spritz->Offset();
        spritz->EnsureSize( sizeof( ContentType));
        spritz->SetOffsetAtEnd();
        ContentType    fileObj;
        if ( obj) 
        {
            fileObj.m_Offset = uint32_t( spritz->Offset() -off);
            Save( spritz, *obj);    
        }
        spritz->SetOffset( off); 
        return fileObj;
    } 

    static ContentType     *Bloom( uint8_t *arr)
    {
        return ( ContentType *) arr;
    }
}; 

//_____________________________________________________________________________________________________________________________

template < typename T, typename... Rest>
struct Cv_MemberCask : public Cv_Cask< T>, public Cv_MemberCask< Rest...> 
{
    typedef Cv_Cask< T>               ItemCask;
    typedef Cv_MemberCask< Rest...>   BaseCask; 

    enum {
        Sz = BaseCask::Sz +1
    };

    typedef typename ItemCask::ContentType    ItemContent;
    typedef typename BaseCask::ContentType    BaseContent;

    struct  ContentType : public BaseContent
    {
        ItemContent    m_Value;

        ContentType(  const BaseContent &t2, const ItemContent &t1)
            : BaseContent( t2), m_Value( t1)
        {}

        BaseContent     *Base( void) { return this; }
    };
        
    static uint32_t         Spread( ContentType *obj) 
    {
        return BaseCask::Spread( obj) +ItemCask::Spread( &obj->m_Value);
    }

    static ContentType     Encase( Cv_Spritz *spritz, const T &obj,  const Rest &... rest)
    {   
        spritz->EnsureSize( sizeof( ContentType)); 
        uint64_t    off = spritz->Offset();
        auto    bc = BaseCask::Encase( spritz,  rest...); 
        spritz->SetOffset( off + uint64_t( &reinterpret_cast< ContentType *>( 0x8)->m_Value) - 0x8);
        auto    ic = ItemCask::Encase( spritz, obj);
        spritz->SetOffset( off);
        return ContentType( bc, ic);
    }
 
    static ContentType     *Bloom( uint8_t *arr)
    {
        return ( ContentType *) arr;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T>
struct Cv_MemberCask< T> : public Cv_Cask< T>
{
    typedef  Cv_Cask< T>            ItemCask;
    enum {
        Sz = 1
    };

    typedef typename ItemCask::ContentType    ItemContent;

    struct  ContentType 
    {
        ItemContent   m_Value;

        ContentType( const ItemContent &t1)
            : m_Value( t1)
        {}
    };

    static uint32_t         Spread( ContentType *obj) 
    {
        return ItemCask::Spread( &obj->m_Value);
    }

    static ContentType     Encase( Cv_Spritz *spritz, const T &obj)
    { 
        return ContentType( ItemCask::Encase( spritz, obj));
    }

    static ContentType     *Bloom( uint8_t *arr)
    {
        return ( ContentType *) arr;
    }
};
 
//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Cask< T, typename Cv_TypeEngage::Exist< typename T::Cask>::Note > :  public T::Cask 
{      
    typedef T                           Type;
    typedef typename T::Cask            Cask;
    typedef typename Cask::ContentType  ContentType;   
};

//_____________________________________________________________________________________________________________________________ 

