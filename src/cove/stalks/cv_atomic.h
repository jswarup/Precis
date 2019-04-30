// cv_atomic.h _____________________________________________________________________________________________________________
#pragma once

#include    "cove/barn/cv_include.h"

//_____________________________________________________________________________________________________________________________

template < typename Stor, typename Atm = std::true_type>
class Cv_Type
{ 
    std::atomic< Stor>          m_Value;
  
public:
    Cv_Type( void) 
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
     
    Cv_Type     &operator=( const Cv_Type &t)
    {
        Set( t.Get());
        return *this;        
    }
};
 
 
//_____________________________________________________________________________________________________________________________

template < typename Stor>
class Cv_Type< Stor, std::false_type>
{
    bool      m_Value;
 
public:
    Cv_Type( void)
        :  m_Value(  0)
    {}
 
    Stor    Get( void) const  {  return Stor( m_Value); }   
    void    Set( Stor t) { m_Value = t; }    

    Cv_Type     &operator=( const Cv_Type &t)
    {
        Set( t.Get());
        return *this;        
    }
    
    
};

//_____________________________________________________________________________________________________________________________
 
//_____________________________________________________________________________________________________________________________
