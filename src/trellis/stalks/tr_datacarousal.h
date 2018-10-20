// tr_datacarousal.h __________________________________________________________________________________________________________
#pragma once
 
#include    <cinttypes>
#include    <cstdio> 
#include    "trellis/stalks/tr_atomic.h"
#include    "cove/silo/cv_dlist.h"

//_____________________________________________________________________________________________________________________________

template < typename Ty> 
class   Tr_DataDock;

//_____________________________________________________________________________________________________________________________ 
 
template < typename Ty> 
class   Tr_DataCarousal
{
public:
    enum {
        Sz = 2048,
    };
    typedef Ty                      Type; 
    typedef Tr_DataDock< Type>      Dock;
private:  
    Type                        m_Buffer[ Sz] alignas( CV_CACHELINE_SIZE);    
    Cv_DLinkList< Dock, true>   m_Docks;
  
public: 
    Tr_DataCarousal( void)   
    {}
        
    ~Tr_DataCarousal()
    {}

    const Type      &Get( uint32_t k) const { return m_Buffer[ k % Sz]; }
    void            Set( uint32_t k, const Type &x) { m_Buffer[ k % Sz] = x; }
       
    void    Append( Dock *dock) { m_Docks.Append( dock); }

    Cv_Couple< uint32_t>    Summon( Dock *dock) const 
    { 
        uint32_t        e = dock->Index(); 
        uint32_t        b = m_Docks.Prev( dock)->Index();
        if (( e < b) || (( e == b) && dock == m_Docks.Head()))
            e += Sz;
        return std::make_tuple( b, e -b);
    }
    
    void       Commit( Dock *dock, uint32_t sz)  {  m_Docks.Prev( dock)->SetIndex( dock->Begin() +sz); }
};

//_____________________________________________________________________________________________________________________________
 
template < typename Ty> 
class   Tr_DataDock : public Cv_DLink< Tr_DataDock< Ty> >
{
    typedef Ty              Type; 

    Tr_Type< uint32_t>      m_Index; 
    Tr_DataCarousal< Ty>    *m_DataCarousal; 
    uint32_t                m_Begin;

public:
    Tr_DataDock( void)
        :   m_DataCarousal( NULL), m_Begin( 0)
    {}
     
    uint32_t    Begin( void) const { return m_Begin; }
    uint32_t    Index( void) const { return m_Index.Get(); }
    void        SetIndex( uint32_t k) {  m_Index.Set( k); }

    void    Setup( Tr_DataCarousal< Ty> *dataCarousal)
    {
        m_DataCarousal = dataCarousal;
        m_DataCarousal->Append( this); 
    } 
    
    uint32_t    Summon( void) 
    { 
        uint32_t    sz = 0;
        std::tie( m_Begin, sz) = m_DataCarousal->Summon( this);
        return sz;
    }
    
    const Type      &Get( uint32_t k) const { return m_DataCarousal->Get(  m_Begin +k); }
    void            Set( uint32_t k, const Type &x) { m_DataCarousal->Set(  m_Begin, x); }

    void       Commit( uint32_t sz) 
    {
        m_DataCarousal->Commit( this, sz);
    }
};

//_____________________________________________________________________________________________________________________________
