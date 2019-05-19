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
template <typename T>
    static void  Save( const T &t, Cv_Spritz *spritz) 
    {
        Cv_Cask< T>().Serialize( t, spritz);
    } 

template <typename T>
    static void  Save( T *t, Cv_Spritz *spritz) 
    {
        Cv_Cask< T*>().Serialize(  t, spritz);
    } 
}; 

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Cask< T, typename Cv_TrivialCopy< T>::Note> : public Cv_SerializeUtils 
{      
    typedef T           Type;
    typedef T           ContentType;  
 

    uint32_t    ObjLen( void) const { return  sizeof( Type); }

    bool        Serialize( const T &obj, Cv_Spritz *spritz)
    {
        bool    res = spritz->Write( &obj, ObjLen());        
        return true;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Cask< Cv_CArr< T> > : public Cv_SerializeUtils 
{
    typedef T           Type; 

    struct  ContentType
    {
        uint64_t    m_Offset;
        uint64_t    m_Size;

        typedef void Copiable; 

    }; 

    uint32_t    ObjLen( void) const { return  sizeof( ContentType); }

    bool        Serialize( const Cv_CArr< Type> &obj, Cv_Spritz *spritz)
    {
        spritz->EnsureSize( ObjLen());
        uint64_t        off = spritz->Offset();
        spritz->SetOffsetAtEnd();
        ContentType    fileObj;
        fileObj.m_Offset = spritz->Offset();
        fileObj.m_Size = obj.Size();
        for ( auto it = obj.Begin(); it != obj.End(); ++it)
            Save( *it, spritz);    
        spritz->SetOffset( off);
        Save( fileObj, spritz);
        return true;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Cask< std::vector< T> > : public Cv_Cask< Cv_CArr< T> > 
{  
    typedef Cv_Cask< Cv_CArr< T>>       BaseCask;
     
    bool    Serialize( const std::vector< Type> &obj, Cv_Spritz *spritz)
    {
        return BaseCask::Serialize( Cv_CArr< T>( obj.size() ? ( T *) &obj[ 0] : NULL, uint32_t( obj.size())), spritz);
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Cask< T *> : public Cv_SerializeUtils 
{  
    typedef T  Type;

    struct  ContentType
    {
        uint64_t    m_Offset;

        typedef void Copiable; 
    };  

    uint32_t    ObjLen( void) const { return  sizeof( ContentType); }

    bool        Serialize( Type *obj, Cv_Spritz *spritz)
    {
        spritz->EnsureSize( ObjLen());
        uint64_t    off = spritz->Offset();
        spritz->SetOffsetAtEnd();

        ContentType    fileObj;
        fileObj.m_Offset = spritz->Offset();  
        Save( *obj, spritz);    
        spritz->SetOffset( off);
        Save( fileObj, spritz);
        return true;
    }
};
 
//_____________________________________________________________________________________________________________________________

template < typename T, typename... Rest>
struct Cv_MemberCask : public Cv_Cask< T>, public Cv_MemberCask< Rest...> 
{
    typedef Cv_Cask< T>               ItemCask;
    typedef Cv_MemberCask< Rest...>   BaseCask; 

    uint32_t    ObjLen( void) const { return  ItemCask::ObjLen() + BaseCask::ObjLen(); }

    bool        Serialize( const T &obj,  const Rest &... rest, Cv_Spritz *spritz)
    { 
        ItemCask::Serialize( obj, spritz);
        BaseCask::Serialize( rest..., spritz);
        return true;  
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T>
struct Cv_MemberCask< T> : public Cv_Cask< T> 
{
    typedef  Cv_Cask< T>            ItemCask;
 
    uint32_t    ObjLen( void) const { return  ItemCask::ObjLen(); }

    bool        Serialize( const T &obj, Cv_Spritz *spritz)
    {
        spritz->EnsureSize( ObjLen());
        ItemCask::Serialize( obj, spritz); 
        return true;  
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Cask< T, typename Cv_TypeEngage::Exist< typename T::Cask>::Note > : public Cv_SerializeUtils 
{      
    typedef T                       Type;
    typedef typename T::Cask        Cask;
    typedef T                       ContentType;  

    uint32_t    ObjLen( void) const { return  Cask().ObjLen(); }

    bool        Serialize( const Type &obj, Cv_Spritz *spritz)
    {       
        Cask().Serialize( obj, spritz);
        return true;
    }
};

//_____________________________________________________________________________________________________________________________

