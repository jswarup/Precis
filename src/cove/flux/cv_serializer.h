// cv_serializer.h ______________________________________________________________________________________________________________
#pragma once
 
#include	"cove/flux/cv_spritz.h"
#include	"cove/silo/cv_array.h"

//_____________________________________________________________________________________________________________________________

template < typename T, typename = void>
struct Cv_Serializer
{ 
    typedef T       Type; 
};

//_____________________________________________________________________________________________________________________________

struct Cv_SerializeUtils
{ 
template <typename T>
    static void  Save( const T &t, Cv_Spritz *spritz) 
    {
        Cv_Serializer< T>       serializer( t);
        serializer.Serialize( spritz);
    } 

template <typename T>
    static void  Save( T *t, Cv_Spritz *spritz) 
    {
        Cv_Serializer< T*>       serializer( t);
        serializer.Serialize( spritz);
    } 
}; 

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Serializer< T, typename Cv_TrivialCopy< T>::Note> : public Cv_SerializeUtils 
{      
    typedef T           Type;
    typedef T           ContentType; 

    const T                 &m_Obj;

    Cv_Serializer( const T &obj)
        : m_Obj( obj)
    {}  

    uint32_t    ObjLen( void) const { return  sizeof( Type); }

    bool    Serialize( Cv_Spritz *spritz)
    {
        bool    res = spritz->Write( &m_Obj, ObjLen());        
        return true;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Serializer< Cv_CArr< T> > : public Cv_SerializeUtils 
{
    typedef T           Type; 

    struct  ContentType
    {
        uint64_t    m_Offset;
        uint64_t    m_Size;

        typedef void Copiable; 

    };

    Cv_CArr< Type>      m_Obj; 

    Cv_Serializer( const Cv_CArr< Type> &obj)
        : m_Obj( obj)
    {} 

    uint32_t    ObjLen( void) const { return  sizeof( ContentType); }

    bool    Serialize( Cv_Spritz *spritz)
    {
        spritz->EnsureSize( ObjLen());
        uint64_t    off = spritz->Offset();
        spritz->SetOffsetAtEnd();
        ContentType    fileObj;
        fileObj.m_Offset = spritz->Offset();
        fileObj.m_Size = m_Obj.Size();
        for ( auto it = m_Obj.Begin(); it != m_Obj.End(); ++it)
            Save( *it, spritz);    
        spritz->SetOffset( off);
        Save( fileObj, spritz);
        return true;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Serializer< std::vector< T> > : public Cv_Serializer< Cv_CArr< T> > 
{  
    typedef Cv_Serializer< Cv_CArr< T>>     Base;
    
    Cv_Serializer( const std::vector< T> &obj)
        : Base( Cv_CArr< T>( obj.size() ? ( T *) &obj[ 0] : NULL, uint32_t( obj.size())))
    {}  
};

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Serializer< T *> : public Cv_SerializeUtils 
{  
    typedef T  Type;

    struct  ContentType
    {
        uint64_t    m_Offset;

        typedef void Copiable; 
    };

    Type        *m_Obj;

    Cv_Serializer( Type *obj)
        : m_Obj( obj)
    {} 

    uint32_t    ObjLen( void) const { return  sizeof( ContentType); }

    bool    Serialize( Cv_Spritz *spritz)
    {
        spritz->EnsureSize( ObjLen());
        uint64_t    off = spritz->Offset();
        spritz->SetOffsetAtEnd();
        ContentType    fileObj;
        fileObj.m_Offset = spritz->Offset();  
        Save( *m_Obj, spritz);    
        spritz->SetOffset( off);
        Save( fileObj, spritz);
        return true;
    }
};
 
//_____________________________________________________________________________________________________________________________

template < typename T, typename... Rest>
struct Cv_MemberSerializer : public Cv_Serializer< T>, public Cv_MemberSerializer< Rest...> 
{
    typedef Cv_Serializer< T>               ItemSerializer;
    typedef Cv_MemberSerializer< Rest...>   BaseSerializer;

    Cv_MemberSerializer( const T &obj,  const Rest &... rest)
        : ItemSerializer( obj), BaseSerializer( rest...)
    {}

    uint32_t    ObjLen( void) const { return  ItemSerializer::ObjLen() + BaseSerializer::ObjLen(); }

    bool    Serialize( Cv_Spritz *spritz)
    {
        spritz->EnsureSize( ObjLen());
        ItemSerializer::Serialize( spritz);
        BaseSerializer::Serialize( spritz);
        return true;  
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T>
struct Cv_MemberSerializer< T> : public Cv_Serializer< T> 
{
    typedef  Cv_Serializer< T>                 ItemSerializer;

    Cv_MemberSerializer( const T &obj)
        : ItemSerializer( obj)
    {}

    uint32_t    ObjLen( void) const { return  ItemSerializer::ObjLen(); }

    bool    Serialize( Cv_Spritz *spritz)
    {
        spritz->EnsureSize( ObjLen());
        ItemSerializer::Serialize( spritz); 
        return true;  
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Serializer< T, typename Cv_TypeEngage::Exist< typename T::Serializer>::Note > : public Cv_SerializeUtils 
{      
    typedef T           Type;
    typedef typename T::Serializer  Serializer;
    typedef T           ContentType; 

    Serializer            m_Serializer;

    Cv_Serializer( const T &obj)
        : m_Serializer( obj)
    {}  


    uint32_t    ObjLen( void) const { return  m_Serializer.ObjLen(); }

    bool    Serialize( Cv_Spritz *spritz)
    {       
        m_Serializer.Serialize( spritz);
        return true;
    }
};

//_____________________________________________________________________________________________________________________________

