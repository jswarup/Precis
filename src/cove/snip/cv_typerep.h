//  cv_typerep.h ___________________________________________________________________________________________________________________
#pragma once

#include 	"cove/barn/cv_aid.h"

//_____________________________________________________________________________________________________________________________  

struct Cv_TypeRep
{
    uint32_t            m_Size;
    std::string         m_Name;
       
    virtual         ~Cv_TypeRep( void);
    
    uint32_t        Size( void) const { return m_Size; }
    Cv_CStr         GetName( void) const { return m_Name; }
    
    void            *Create( void)  const { return Construct( new uint8_t[ m_Size]);}
    void            Destroy( void *ptr) const { Destruct( ptr); delete [] ( uint8_t *) ptr; } 
    
    virtual void    *Construct( void *ptr) const = 0;
    virtual void    Destruct( void *ptr) const = 0;
    
    virtual void    *Copy( void *from, void *to) const = 0;  
     
};

//_____________________________________________________________________________________________________________________________  

template < typename Class>
struct Cv_TypeBroker : public Cv_TypeRep
{
     
    Cv_TypeBroker( void)
    {
        m_Size = sizeof( Class);
        m_Name = Class::Name;
    } 
     
    
    virtual void    *Construct( void *ptr) const { return new ( ptr) Class(); }
    virtual void    Destruct( void *ptr) const  { return static_cast< Class>( ptr)->Class::~Class(); }
    
    virtual void    *Copy( void *from, void *to) const { return static_cast< Class>( to) = *static_cast< Class>( from); }
    
};

//_____________________________________________________________________________________________________________________________  
