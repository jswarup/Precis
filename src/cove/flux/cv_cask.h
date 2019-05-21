// cv_cask.h ______________________________________________________________________________________________________________
#pragma once
 
#include	"cove/flux/cv_spritz.h"
#include	"cove/silo/cv_array.h"

//_____________________________________________________________________________________________________________________________

template < typename T, typename = void>
struct Cv_Cask
{ 
    typedef T       Type; 
};

//_____________________________________________________________________________________________________________________________

struct Cv_SerializeUtils
{ 
template < typename Spritz, typename T>
    static void  Save( Spritz *spritz, const T &t) 
    {
        auto    cnt = Cv_Cask< T>().Encase( spritz, t);
        bool    res = spritz->Write( &cnt, sizeof( cnt));  
    } 
/*
template < typename Spritz, typename T>
    static void  Save( Spritz *spritz, T *t) 
    {
        Cv_Cask< T*>().Encase( spritz, t);
    } 
*/
}; 

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Cask< T, typename Cv_TrivialCopy< T>::Note> : public Cv_SerializeUtils 
{      
    typedef T           Type;
    typedef T           ContentType;  
 

    ContentType         Encase( Cv_Spritz *spritz, const T &obj)
    { 
        return obj;
    }
 
    ContentType     *Bloom( const Cv_CArr< uint8_t> &arr)
    {
        return ( ContentType *) arr.Ptr();
    } 
};

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Cask< Cv_CArr< T> > : public Cv_SerializeUtils 
{
    typedef T                               Type; 
    typedef  Cv_Cask< Type>                 SubCask;
    typedef typename SubCask::ContentType   SubContent;

    struct  ContentType
    {
        uint32_t    m_Offset;
        uint32_t    m_Size;

        typedef void Copiable;  
 
        Cv_CArr< SubContent>     Value( const Cv_CArr< uint8_t> &arr)
        {
            return Cv_CArr< SubContent>( ( SubContent *) ( arr.Begin() + m_Offset), m_Size);
        }
 
    }; 

    ContentType        Encase( Cv_Spritz *spritz, const Cv_CArr< Type> &obj)
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
 
    ContentType     *Bloom( const Cv_CArr< uint8_t> &arr)
    {
        return ( ContentType *) arr.Begin();
    }
};

//_____________________________________________________________________________________________________________________________
/*
template < typename T> 
struct Cv_Cask< T, typename Cv_TrivialCopy< T>::Note> : public Cv_SerializeUtils 
{      
    typedef T           Type;
    typedef T           ContentType;  
 

    uint32_t    ObjLen( void) const { return  sizeof( Type); }

    bool        Serialize( Cv_Spritz *spritz, const T &obj)
    {
        bool    res = spritz->Write( &obj, ObjLen());        
        return true;
    }

template < typename Spritz>
    ContentType Bloom( Spritz *spritz)
    {
        ContentType     obj;
        bool    res = spritz->Read( &obj, ObjLen()); 
        return obj;
    }


    ContentType *Blossom( Cv_CArr< uint8_t> *arr)
    {
        return ( ContentType *) arr->Ptr();
    }

};

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Cask< Cv_CArr< T> > : public Cv_SerializeUtils 
{
    typedef T                               Type; 
    typedef  Cv_Cask< Type>                 SubCask;
    typedef typename SubCask::ContentType   SubContent;

    struct  ContentType
    {
        uint64_t    m_Offset;
        uint64_t    m_Size;

        typedef void Copiable; 

        SubContent     Content( Cv_Spritz *spritz, uint32_t k)
        {
            uint32_t        objLen = SubCask().ObjLen();
            uint64_t        off = spritz->Offset();
            spritz->SetOffset( m_Offset + k * SubCask().ObjLen());
            SubContent      obj;
            bool    res = spritz->Read( &obj, ObjLen());   
            spritz->SetOffset( off);
            return obj;
        }
    }; 

    uint32_t    ObjLen( void) const { return  sizeof( ContentType); }

    bool        Serialize( Cv_Spritz *spritz, const Cv_CArr< Type> &obj)
    {
        spritz->EnsureSize( ObjLen());
        uint64_t        off = spritz->Offset();
        spritz->SetOffsetAtEnd();
        ContentType    fileObj;
        fileObj.m_Offset = spritz->Offset();
        fileObj.m_Size = obj.Size();
        for ( auto it = obj.Begin(); it != obj.End(); ++it)
            Save( spritz, *it);    
        spritz->SetOffset( off);
        Save( spritz, fileObj);
        return true;
    }

template < typename Spritz>
    ContentType     Bloom( Spritz *spritz)
    {
        ContentType     obj;
        bool    res = spritz->Read( &obj, ObjLen()); 
        return obj;
    }

    ContentType     *Blossom( Cv_CArr< uint8_t> *arr)
    {
        return ( ContentType *) arr->Ptr();
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Cask< std::vector< T> > : public Cv_Cask< Cv_CArr< T> > 
{  
    typedef Cv_Cask< Cv_CArr< T>>       BaseCask;
     
    bool    Serialize( Cv_Spritz *spritz, const std::vector< T> &obj)
    {
        return BaseCask::Serialize( spritz, Cv_CArr< T>( obj.size() ? ( T *) &obj[ 0] : NULL, uint32_t( obj.size())));
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Cask< T *> : public Cv_SerializeUtils 
{  
    typedef T                               Type;
    typedef Cv_Cask< Type>                  SubCask;
    typedef typename SubCask::ContentType   SubContent;

    struct  ContentType
    {
        uint64_t        m_Offset;

        typedef void    Copiable; 

        SubContent      Content( Cv_Spritz *spritz)
        {
            uint32_t        objLen = SubCask().ObjLen();
            uint64_t    off = spritz->Offset();
            spritz->SetOffset( m_Offset);
            SubContent      obj;
            bool            res = spritz->Read( &obj, ObjLen()); 
            spritz->SetOffset( off);
            return obj;
        }
    };  

    uint32_t    ObjLen( void) const { return  sizeof( ContentType); }

    bool        Serialize( Cv_Spritz *spritz, Type *obj)
    {
        spritz->EnsureSize( ObjLen());
        uint64_t    off = spritz->Offset();
        spritz->SetOffsetAtEnd();

        ContentType    fileObj;
        fileObj.m_Offset = spritz->Offset();  
        Save( spritz, *obj);    
        spritz->SetOffset( off);
        Save( spritz, fileObj);
        return true;
    }

template < typename Spritz>
    ContentType     Bloom( Spritz *spritz)
    {
        ContentType     obj;
        bool    res = spritz->Read( &obj, ObjLen()); 
        return obj;
    }
    
    ContentType     *Blossom( Cv_CArr< uint8_t> *arr)
    {
        return ( ContentType *) arr->Ptr();
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
    
        ContentType( const ItemContent &t1, const BaseContent &t2)
            : m_Value( t1), BaseContent( t2)
        {}
    };

    uint32_t        ObjLen( void) const { return  ItemCask::ObjLen() + BaseCask::ObjLen(); }

    bool            Serialize( Cv_Spritz *spritz, const T &obj,  const Rest &... rest)
    { 
        BaseCask::Serialize( spritz,  rest...);
        ItemCask::Serialize( spritz, obj);
        return true;  
    }

template < typename Spritz>
    ContentType     Bloom(  Spritz *spritz)
    {
        ContentType     obj( ItemCask::Bloom( spritz), BaseCask::Bloom( spritz)); 
        return obj;
    }

    ContentType     *Blossom( Cv_CArr< uint8_t> *arr)
    {
        return  ( ContentType *) arr->Ptr();
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

    uint32_t    ObjLen( void) const { return  ItemCask::ObjLen(); }
 
    bool        Serialize( Cv_Spritz *spritz, const T &obj)
    {
        spritz->EnsureSize( ObjLen());
        ItemCask::Serialize( spritz, obj); 
        return true;  
    }

template < typename Spritz>
    ContentType     Bloom( Spritz *spritz)
    {
        ContentType     obj( ItemCask::Bloom( spritz)); 
        return obj;
    }

    ContentType     *Blossom( Cv_CArr< uint8_t> *arr)
    {
        return arr->Ptr();
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Cask< T, typename Cv_TypeEngage::Exist< typename T::Cask>::Note > : public Cv_SerializeUtils 
{      
    typedef T                           Type;
    typedef typename T::Cask            Cask;
    typedef typename Cask::ContentType  ContentType;  

    uint32_t    ObjLen( void) const { return  Cask().ObjLen(); }

    bool        Serialize( Cv_Spritz *spritz, const Type &obj)
    {       
        Cask().Serialize( spritz, obj);
        return true;
    }

template < typename Spritz>
    ContentType     Bloom( Spritz *spritz)
    {
        return Cask().Bloom( spritz); 
    }

    ContentType     *Blossom( Cv_CArr< uint8_t> *arr)
    {
        return ( ContentType *) Cask().Blossom( arr); 
    }
};
*/
//_____________________________________________________________________________________________________________________________

