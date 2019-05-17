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
}; 

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Serializer< T, typename Cv_TrivialCopy< T>::Note> : public Cv_SerializeUtils 
{      
    typedef T           Type;
    typedef T           FileType; 

    const T                 &m_Obj;

    Cv_Serializer( const T &obj)
        : m_Obj( obj)
    {}  

    FileType    Serialize( Cv_Spritz *spritz)
    {
        bool    res = spritz->Write( &m_Obj, sizeof( Type));        
        return m_Obj;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Serializer< Cv_CArr< T> > : public Cv_SerializeUtils 
{
    typedef T           Type; 

    struct  FileType
    {
        uint64_t    m_Offset;
        uint64_t    m_Size;

        typedef void Copiable; 

    };

    Cv_CArr< Type>      m_Obj; 

    Cv_Serializer( const Cv_CArr< Type> &obj)
        : m_Obj( obj)
    {} 

    uint32_t    ObjLen( void) const { return  sizeof( FileType); }

    FileType    Serialize( Cv_Spritz *spritz)
    {
        spritz->EnsureSize( ObjLen());
        uint64_t    off = spritz->Offset();
        spritz->SetOffsetAtEnd();
        FileType    fileObj;
        fileObj.m_Offset = spritz->Offset();
        fileObj.m_Size = m_Obj.Size();
        for ( auto it = m_Obj.Begin(); it != m_Obj.End(); ++it)
            Save( *it, spritz);    
        spritz->SetOffset( off);
        Save( fileObj, spritz);
        return fileObj;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Serializer< std::vector< T> > : public Cv_Serializer< Cv_CArr< T> > 
{  
    typedef Cv_Serializer< Cv_CArr< T>>     Base;
    
    Cv_Serializer( const std::vector< Type> &obj)
        : Base( Cv_CArr< T>( ( T *) &obj[ 0], uint32_t( obj.size())))
    {}  
};

//_____________________________________________________________________________________________________________________________


 
//_____________________________________________________________________________________________________________________________

