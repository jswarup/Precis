// tr_datacarousal.h __________________________________________________________________________________________________________
#pragma once
 
#include    <cinttypes>
#include    <cstdio> 
#include    "cove/stalks/cv_atomic.h"
#include    "cove/silo/cv_dlist.h"

//_____________________________________________________________________________________________________________________________

template < typename Ty> 
class   Cv_DataDock;

//_____________________________________________________________________________________________________________________________ 
 
template < typename Dock> 
class   Cv_DataCarousal
{
public:
    enum {
        Sz = 2048,
    };
    typedef typename Dock::Type     Type; 

private:  
    Type                        m_Buffer[ Sz] alignas( CV_CACHELINE_SIZE);      // circular buffer
    Cv_DLinkList< Dock, true>   m_Docks;                                        // docks for data-transfers
  
public: 
    Cv_DataCarousal( void)   
    {}
        
    ~Cv_DataCarousal()
    {}

    const Type      &Get( uint32_t k) const { return m_Buffer[ k % Sz]; }
    void            Set( uint32_t k, const Type &x) { m_Buffer[ k % Sz] = x; }
       
    void        AppendDock( Dock *dock) { m_Docks.Append( dock); }

    Cv_Couple< uint32_t>    SummonDock( Dock *dock) const 
    { 
        uint32_t        e = dock->Index(); 
        uint32_t        b = m_Docks.Prev( dock)->Index();
        if (( e < b) || (( e == b) && dock == m_Docks.Head()))
            e += Sz;
        return std::make_tuple( b, e -b);
    }
    
    void        Commit( Dock *dock, uint32_t index)  {  m_Docks.Prev( dock)->SetIndex( index); }
    
    bool    IsTail( Dock *dock) 
    {
        return m_Docks.Tail() == dock;
    }
};

//_____________________________________________________________________________________________________________________________
 
template < typename Ty> 
class   Cv_DataDock : public Cv_DLink< Cv_DataDock< Ty> >
{
public:
    typedef Ty                      Type; 
    typedef Cv_DataDock< Type>      This; 

    typedef Cv_DataCarousal< This>  DataCarousal;

protected:
    Cv_Type< uint32_t>      m_Index; 
    DataCarousal            *m_DataCarousal; 

public:
    struct  Wharf
    {
        Cv_DataDock     *m_Dock;
        uint32_t        m_Begin;
        uint32_t        m_Sz;
        bool            m_Tail;

        Wharf( Cv_DataDock *dock)
            : m_Dock( dock), m_Begin( 0), m_Sz( 0),  m_Tail( m_Dock->IsTail())
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

    Cv_DataDock( void)
        :   m_DataCarousal( NULL)
    {}
     
    uint32_t    Index( void) const { return m_Index.Get(); }
    void        SetIndex( uint32_t k) {  m_Index.Set( k); }

    void    Connect( DataCarousal *dataCarousal)
    {
        m_DataCarousal = dataCarousal;
        m_DataCarousal->AppendDock( this); 
    }   

    bool    IsTail( void) 
    {
        return m_DataCarousal->IsTail( this);
    }
};

//_____________________________________________________________________________________________________________________________

template < typename Ty> 
class   Cv_DataCreek : public Cv_DataDock< Ty>
{
public:
    typedef Cv_DataDock< Ty>                Base;  
    typedef typename Base::DataCarousal     DataCarousal;

protected:
    DataCarousal                    m_DataCarousal;

public:
    Cv_DataCreek( void)
    {
        Base::Connect( &m_DataCarousal);
    } 

    DataCarousal        *Carousal( void) { return &m_DataCarousal; }

};

//_____________________________________________________________________________________________________________________________
