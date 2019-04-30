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
    Type                        m_Buffer[ Sz] alignas( CV_CACHELINE_SIZE);      // circular buffer
    Cv_DLinkList< Dock, true>   m_Docks;                                        // docks for data-transfers
  
public: 
    Tr_DataCarousal( void)   
    {}
        
    ~Tr_DataCarousal()
    {}

    const Type      &Get( uint32_t k) const { return m_Buffer[ k % Sz]; }
    void            Set( uint32_t k, const Type &x) { m_Buffer[ k % Sz] = x; }
       
    void    AppendDock( Dock *dock) { m_Docks.Append( dock); }

    Cv_Couple< uint32_t>    SummonDock( Dock *dock) const 
    { 
        uint32_t        e = dock->Index(); 
        uint32_t        b = m_Docks.Prev( dock)->Index();
        if (( e < b) || (( e == b) && dock == m_Docks.Head()))
            e += Sz;
        return std::make_tuple( b, e -b);
    }
    
    void       Commit( Dock *dock, uint32_t index)  {  m_Docks.Prev( dock)->SetIndex( index); }
};

//_____________________________________________________________________________________________________________________________
 
template < typename Ty> 
class   Tr_DataDock : public Cv_DLink< Tr_DataDock< Ty> >
{
public:
    typedef Ty                      Type; 
    typedef Tr_DataCarousal< Type>  DataCarousal;

protected:
    Tr_Type< uint32_t>      m_Index; 
    DataCarousal            *m_DataCarousal; 

public:
    struct  Wharf
    {
        Tr_DataDock     *m_Dock;
        uint32_t        m_Begin;
        uint32_t        m_Sz;

        Wharf( Tr_DataDock *dock)
            : m_Dock( dock), m_Begin( 0), m_Sz( 0)
        { 
            std::tie( m_Begin, m_Sz) = m_Dock->m_DataCarousal->SummonDock( m_Dock);
        }

        ~Wharf( void)
        {
            if ( m_Sz)
                m_Dock->m_DataCarousal->Commit( m_Dock, m_Begin +m_Sz);
        }

        uint32_t        Begin( void) const { return m_Begin; }
        uint32_t        Size( void) const { return m_Sz; }
        void            SetSize( uint32_t sz) { m_Sz = sz; } 

        const Type      &Get( uint32_t k) const { return m_Dock->m_DataCarousal->Get(  m_Begin +k); }
        void            Set( uint32_t k, const Type &x) { m_Dock->m_DataCarousal->Set(  m_Begin +k, x); }
    };

    Tr_DataDock( void)
        :   m_DataCarousal( NULL)
    {}
     
    uint32_t    Index( void) const { return m_Index.Get(); }
    void        SetIndex( uint32_t k) {  m_Index.Set( k); }

    void    Connect( Tr_DataCarousal< Ty> *dataCarousal)
    {
        m_DataCarousal = dataCarousal;
        m_DataCarousal->AppendDock( this); 
    }   
};

//_____________________________________________________________________________________________________________________________

template < typename Ty> 
class   Tr_DataCreek : public Tr_DataDock< Ty>
{
public:
    typedef Tr_DataDock< Ty>        Base;  
    
protected:
    DataCarousal                    m_DataCarousal;

public:
    Tr_DataCreek( void)
    {
        Base::Connect( &m_DataCarousal);
    } 

    DataCarousal        *Carousal( void) { return &m_DataCarousal; }

};

//_____________________________________________________________________________________________________________________________
