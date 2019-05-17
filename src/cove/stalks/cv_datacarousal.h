// tr_datacarousal.h __________________________________________________________________________________________________________
#pragma once
 
#include    <cinttypes>
#include    <cstdio> 
#include    "cove/stalks/cv_atomic.h"
#include    "cove/silo/cv_dlist.h"
#include    "cove/silo/cv_freestore.h" 

//_____________________________________________________________________________________________________________________________

template < typename Ty, uint32_t CarousalSz> 
class   Cv_DataDock;

//_____________________________________________________________________________________________________________________________ 
 
template < typename Dock, uint32_t CarouSz> 
class   Cv_DataCarousal
{
public:
    enum {
        CarousalSz = CarouSz,
    };
    typedef typename Dock::Type     Type; 

private:  
    Type                        m_Buffer[ CarousalSz] alignas( CV_CACHELINE_SIZE);      // circular buffer
    Cv_DLinkList< Dock, true>   m_Docks;                                        // docks for data-transfers
  
public: 
    Cv_DataCarousal( void) 
    {}
        
    ~Cv_DataCarousal()
    {}

    const Type      &Get( uint32_t k) const  {  return m_Buffer[ k % CarousalSz];  }
    void            Set( uint32_t k, const Type &x)  {  m_Buffer[ k % CarousalSz] = x;  }
       
    void            AppendDock( Dock *dock) { m_Docks.Append( dock); }

    Cv_Couple< uint32_t>    SummonDock( Dock *dock) const 
    { 
        uint32_t        b = dock->Index(); 
        uint32_t        e = m_Docks.Prev( dock)->Index();
        //if ( b == CV_UINT32_MAX)
        //    return std::make_tuple( b, b);
        if (( e < b) || (( e == b) && ( dock == m_Docks.Head())))
            e += CarousalSz;
        return std::make_tuple( b, e -b);
    } 
    
    bool            IsTail( const Dock *dock)  { return m_Docks.Tail() == dock; }
    
    bool            IsHead( const Dock *dock)  { return m_Docks.Head() == dock; }

    Dock            *Prev( const Dock *dock)  { return m_Docks.Prev( dock); } 
};

//_____________________________________________________________________________________________________________________________
 
template < typename Ty, uint32_t CarousalSz> 
class   Cv_DataDock : public Cv_DLink< Cv_DataDock< Ty, CarousalSz> >
{
public:
    typedef Ty                                      Type; 
    typedef Cv_DataDock< Type, CarousalSz>          This; 

    typedef Cv_DataCarousal< This, CarousalSz>      DataCarousal;

protected:
    Cv_Type< uint32_t>              m_Index; 
    Cv_Type< uint32_t>              m_CloseFlg;
    DataCarousal                    *m_DataCarousal; 

public:
    struct  Wharf
    {
        typedef This    Dock; 

        Dock            *m_Dock;
        uint32_t        m_Begin;
        uint32_t        m_Sz; 

        Wharf( Dock *dock)
            : m_Dock( dock), m_Begin( 0), m_Sz( 0) 
        { 
            std::tie( m_Begin, m_Sz) = m_Dock->m_DataCarousal->SummonDock( m_Dock); 
        }

        ~Wharf( void)
        {
            if ( m_Sz)
                m_Dock->SetIndex( m_Begin +m_Sz);
        }
        
        bool            IsTail( void) const { return m_Dock->IsTail(); }
        uint32_t        Begin( void) const { return m_Begin; }
        uint32_t        Size( void) const { return m_Sz; }
        void            SetSize( uint32_t sz) { m_Sz = sz; } 
        bool            SetClose( void)  {  m_Sz = 0;  return  m_Dock->SetClose(); } 
        bool            IsClose( void)  { return !m_Dock->m_DataCarousal->IsHead( m_Dock) && m_Dock->m_DataCarousal->Prev( m_Dock)->IsClose(); }  
        const Type      &Get( uint32_t k) const { return m_Dock->m_DataCarousal->Get(  m_Begin +k); }
        void            Set( uint32_t k, const Type &x) { m_Dock->m_DataCarousal->Set(  m_Begin +k, x); }
    };

    Cv_DataDock( void)
        :   m_CloseFlg( 0), m_DataCarousal( NULL)
    {}
     
    uint32_t    Index( void) const { return m_Index.Get(); }
    void        SetIndex( uint32_t k) {  m_Index.Set( k % DataCarousal::CarousalSz); }

    void    Connect( DataCarousal *dataCarousal)
    {
        m_DataCarousal = dataCarousal;
        m_DataCarousal->AppendDock( this); 
    }   

    bool    IsTail( void)  const
    {
        return m_DataCarousal->IsTail( this);
    }

    bool    SetClose( void)
    {
        m_CloseFlg.Set( 1);
        return true;
    }

    bool    IsClose( void)
    {
        return !!m_CloseFlg.Get();
    }
};

//_____________________________________________________________________________________________________________________________

template < typename Ty, uint32_t CarousalSz> 
class   Cv_DataCreek : public Cv_DataDock< Ty, CarousalSz>
{
public:
    typedef Cv_DataDock< Ty, CarousalSz>    Base;  
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

template < typename DGram, uint32_t CacheSz = 128, uint32_t StoreSz = 4096, uint32_t CarousSz = 2048>
struct Sg_DataSink
{
    enum {
        CarousalSz = CarousSz
    };
    typedef DGram                                       Datagram;
    typedef Cv_FreeStore< Datagram, uint16_t, StoreSz>  DataStore;
    typedef Cv_FreeCache< CacheSz, DataStore>           DataCache;

    typedef Cv_DataCreek< Datagram *, CarousalSz>       Dock;

    Dock                    m_Dock; 
    DataStore               m_DataStore;
    DataCache               m_DataCache; 

    struct  Wharf : public Dock::Wharf
    {
        Sg_DataSink     *m_Port;

        Wharf( Sg_DataSink *port)
            : m_Port( port), Dock::Wharf( &port->m_Dock)
        {}

        uint32_t        ProbeSzFree( uint32_t szExpect) { return m_Port->m_DataCache.ProbeSzFree( szExpect); }
        Datagram        *AllocFree( void) { return m_Port->m_DataCache.AllocFree(); }    
        void            Discard( Datagram *datagram ) { m_Port->m_DataCache.Discard( datagram); }
    };

    Sg_DataSink( void) 
        : m_DataStore( 0x7), m_DataCache( &m_DataStore)
    {}
};

//_____________________________________________________________________________________________________________________________

template < typename Sink>
struct Sg_DataSource 
{ 
    typedef typename Sink::DataCache            DataCache;
    typedef typename Sink::Datagram             Datagram;

    typedef Cv_DataDock<Datagram *, Sink::CarousalSz> Dock;

    Dock                    m_Dock;  
    DataCache               m_DataCache; 

    struct  Wharf : public Dock::Wharf
    {
        Sg_DataSource     *m_Port;

        Wharf( Sg_DataSource *port)
            : m_Port( port), Dock::Wharf( &port->m_Dock)
        {}

        uint32_t        ProbeSzFree( uint32_t szExpect) { return m_Port->m_DataCache.ProbeSzFree( szExpect); }
        Datagram        *AllocFree( void) { return m_Port->m_DataCache.AllocFree(); }    
        void            Discard( Datagram *datagram ) { m_Port->m_DataCache.Discard( datagram); }
    };

    Sg_DataSource( void)  
        : m_DataCache( NULL)
    {}

    void    Connect( Sink *provider)
    {
        m_DataCache.SetStore( &provider->m_DataStore);
        m_Dock.Connect( provider->m_Dock.Carousal());
    }
};

//_____________________________________________________________________________________________________________________________
