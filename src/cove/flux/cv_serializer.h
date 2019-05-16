// cv_serializer.h ______________________________________________________________________________________________________________
#pragma once
 
#include	"cove/flux/cv_spritz.h"
//_____________________________________________________________________________________________________________________________

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
    static auto  Serialize( T *t, Cv_Spritz *spritz) 
    {
        Cv_Serializer< T>       serializer( t);
        return serializer.Save( spritz);
    }

};


//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Serializer< T, typename Cv_TrivialCopy< T>::Note> : public Cv_SerializeUtils
{      
    typedef T                   Type;
    typedef T                   FileType; 

    Type                *m_Obj;

    Cv_Serializer( Type *obj)
        : m_Obj( obj)
    {} 
 
    FileType    Save( Cv_Spritz *spritz)
    {
        bool    res = spritz->Write( m_Obj, sizeof( Type));        
        return *m_Obj;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T> 
struct Cv_Serializer< std::vector< T> > : public Cv_SerializeUtils
{ 
    typedef T                   Type; 

    struct  FileType
    {
        uint64_t    m_Offset;
        uint32_t    m_Size;
        
        typedef void Copiable;
        
        FileType( uint64_t off, uint32_t sz)
            :   m_Offset( off), m_Size( sz)
        {}
    };

    std::vector< Type>     *m_Obj;

    Cv_Serializer( std::vector< Type> *obj)
        : m_Obj( obj)
    {} 
 

    FileType    Save( Cv_Spritz *spritz)
    {
        uint64_t                        off = spritz->Offset();
        FileType                        fileObj( off, uint32_t( m_Obj->size()));
        Serialize( &fileObj, spritz);
        off = spritz->Offset();
        spritz->SetOffsetAtEnd();
        for ( auto it = m_Obj->begin(); it != m_Obj->end(); ++it)
            Serialize( &*it, spritz);    
        spritz->SetOffset( off);
        return fileObj;
    }
};

//_____________________________________________________________________________________________________________________________

