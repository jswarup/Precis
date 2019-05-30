// cv_atomic.h _____________________________________________________________________________________________________________
#pragma once

#include    "cove/barn/cv_include.h"

//_____________________________________________________________________________________________________________________________

template < typename Stor, typename Atm = std::true_type>
class Cv_Type
{ 
    std::atomic< Stor>          m_Value;
  
public:
    Cv_Type( const Stor &value = Stor()) 
    :  m_Value( value)
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
    
    void            Incr( void)  { ++m_Value;  } 

    void            Decr( void)  { --m_Value;  }

template < typename T>
    Cv_Type     &operator+=( const T &t) 
    {
        m_Value += t;
        return *this;
    }

    Stor    Diff( const Cv_Type &t) 
    {
        return m_Value -t.m_Value;
    }
};
 
 
//_____________________________________________________________________________________________________________________________

template < typename Stor>
class Cv_Type< Stor, std::false_type>
{
    Stor      m_Value;
 
public:
    Cv_Type( const Stor &value = Stor()) 
        :  m_Value( value)
    {} 
 
    Stor    Get( void) const  {  return Stor( m_Value); }   
    void    Set( Stor t) { m_Value = t; }    

    Cv_Type     &operator=( const Cv_Type &t)
    {
        Set( t.Get());
        return *this;        
    }
    
    void            Incr( void)  { ++m_Value;  } 

    void            Decr( void)  { --m_Value;  }

 template < typename T>
    Cv_Type     &operator+=( const T &t) 
    {
        m_Value += t;
        return *this;
    }
    
    Stor    Diff( const Cv_Type &t) 
    {
        return m_Value -t.m_Value;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename Atm>
class Cv_Spinlock
{     
    std::atomic_flag m_Flag; 

public:
    Cv_Spinlock( void)  
        : m_Flag() 
    {} 

    void Lock()
    { 
        while ( m_Flag.test_and_set())
        {}      
    }

    void Unlock()
    { 
        m_Flag.clear(); 
    }

    struct Guard
    {
        Cv_Spinlock     *m_Lock;

        Guard( Cv_Spinlock *lck)
            : m_Lock( lck)
        {
            m_Lock->Lock();
        }

        ~Guard()
        {
            m_Lock->Unlock();
        }                
    };  
};

//_____________________________________________________________________________________________________________________________

template <>
class Cv_Spinlock<std::false_type>
{       

public:
    Cv_Spinlock<std::false_type>( void)  
    {} 

    void Lock()
    {  
    }

    void Unlock()
    { 
    }

    struct Guard
    { 
        Guard( Cv_Spinlock<std::false_type> *) 
        { 
        }

        ~Guard()
        { 
        }                
    };  
};
//_____________________________________________________________________________________________________________________________
