// tr_atomic.h _____________________________________________________________________________________________________________
#pragma once

#include    "trellis/tenor/tr_include.h"

//_____________________________________________________________________________________________________________________________

template < typename Stor, typename Atm = std::true_type>
class Tr_Type
{ 
    std::atomic< Stor>          m_Value;
  
public:
    Tr_Type( void) 
    :  m_Value( 0)
    {} 
  
    Stor    Get( void) const 
    {  
        return Stor( m_Value); 
    }  
    
    void    Set( Stor t)
    {  
        m_Value.store( t, std::memory_order_relaxed); 
    }    
     
    Tr_Type     &operator=( const Tr_Type &t)
    {
        Set( t.Get());
        return *this;        
    }
};
 
 
//_____________________________________________________________________________________________________________________________

template < typename Stor>
class Tr_Type< Stor, std::false_type>
{
    bool      m_Value;
 
public:
    Tr_Type( void)
        :  m_Value(  0)
    {}
 
    Stor    Get( void) const  {  return Stor( m_Value); }   
    void    Set( Stor t) { m_Value = t; }    

    Tr_Type     &operator=( const Tr_Type &t)
    {
        Set( t.Get());
        return *this;        
    }
    
    
};

//_____________________________________________________________________________________________________________________________
 
//_____________________________________________________________________________________________________________________________
