// cv_freestore.h ____________________________________________________________________________________________________________ 
#pragma once

#include    "cove/silo/cv_stack.h" 
#include    "trellis/stalks/tr_atomic.h"

//_____________________________________________________________________________________________________________________________
// Memory dispensor of Type X, IndexType DStor, of Max ppol-size Mx

template < typename Store,  class X, typename DStor, uint32_t Mx>
struct Cv_BaseStore
{  
    enum {
        StorMax = Mx 
    };
     
    typedef std::true_type          Atomic;
    typedef DStor                   Stor;
    typedef X                       Type; 
    typedef Cv_Array< Stor, Mx>     ObjStack;                                  
                                 
    ObjStack                        m_Store;                                // Stack of free Indexes into Page at each object location
    Cv_Spinlock<Atomic>             m_Lock;                                 // SpinLock to be used for concurrent access   
    uint32_t                        m_StoreId;
    
    Store                           *FetchStore( void) const { return ( Store *) this; }
    uint8_t                         *Page( void) const { return FetchStore()->Page(); }
    Cv_BaseStore( uint32_t storeId)
        :   m_StoreId( storeId)
    {       
        m_Store.m_SzFill = StorMax;                                          // initially, all are free and indexes by integer-sequence
        for ( uint32_t i = 0; i < StorMax; ++i)
            m_Store[ i] = i;     
    }
    
    ~Cv_BaseStore( void)
    {}
     
    
template < typename CacheStore>
    void        AllocBulk( CacheStore *cacheStore)                          // allocate in bulk : caller needs to SpinLock if needed
    {
       m_Store.TransferTo( cacheStore);  
    } 

template < typename CacheStore>
    void        DiscardBulk( CacheStore *cacheStore)                        // discard in bulk : caller needs to SpinLock if needed
    {
        cacheStore->TransferTo( &m_Store); 
    }
 
    Type        *Alloc( void)                                               // allocaate one : caller needs to SpinLock if needed
    { 
        return m_Store.m_SzFill ? FetchStore()->GetAt( m_Store[ --m_Store.m_SzFill]) : NULL;
    }
    
    void        Discard( Type *x)                                           // discard one : caller needs to SpinLock if needed
    { 
        m_Store[ m_Store.m_SzFill++] =  FetchStore()->MapId( x); 
    } 
};

//_____________________________________________________________________________________________________________________________
// Memory dispensor of Type X, IndexType DStor, of Max ppol-size Mx

template < class X, typename DStor, uint32_t Mx, uint32_t ObjSz>
struct Cv_FreeStore : public Cv_BaseStore< Cv_FreeStore< X, DStor, Mx,  ObjSz>, X, DStor, Mx>
{   
    typedef Cv_BaseStore< Cv_FreeStore< X, DStor, Mx,  ObjSz>, X, DStor, Mx>    Base;
    
    uint8_t                         m_Page[ Mx * ObjSz]; 
    
    Cv_FreeStore( uint32_t storeId)
        :  Base( storeId)
    {} 
    
    X           *GetAt( uint32_t id) const                              // get pointer for index
    { 
        XD_SANITY_ASSERT( id < Mx)
        return reinterpret_cast< X *>( const_cast< uint8_t *>( &m_Page[ id * ObjSz])); 
    }
    
    DStor        MapId( const X *x) const                                // get index for pointer
    {   
        DStor        id( ( reinterpret_cast< const uint8_t *>( x) - m_Page) /ObjSz);
        XD_SANITY_ASSERT( id < Mx)
        return id; 
    } 
};
 

//_____________________________________________________________________________________________________________________________
// Memory dispensor of Type X, IndexType DStor, of Max ppol-size Mx

template < class X, typename DStor, uint32_t Mx, uint32_t AlignSz = XD_CACHELINE_SIZE>
struct Cv_MemStore  : public Cv_BaseStore< Cv_MemStore< X, DStor, Mx,  AlignSz>, X, DStor, Mx >
{   
    typedef Cv_BaseStore< Cv_MemStore< X, DStor, Mx,  AlignSz>, X, DStor, Mx>    Base;
    
    enum { 
        ObjSz = Cv_Utils::Align< AlignSz>( sizeof( X)),         //  Align to AlignSz-Byte boundry.. 
    }; 
    
    uint8_t                         *m_Page;
    uint32_t                        m_ObjSz;                            //  Align to AlignSz-Byte boundry.. 
     
    
    Cv_MemStore( uint32_t storeId, uint8_t *page, uint32_t objSz = XD_UINT32_MAX)
        : Base( storeId), m_Page( page), m_ObjSz( objSz == XD_UINT32_MAX ? uint32_t( ObjSz) : objSz)
    { }
     
    
    X           *GetAt( uint32_t id) const                              // get pointer for index
    { 
        XD_SANITY_ASSERT( id < Mx)
        return reinterpret_cast< X *>( const_cast< uint8_t *>( &m_Page[ id * m_ObjSz])); 
    }
    
    DStor        MapId( const X *x) const                                // get index for pointer
    {   
        DStor        id( ( reinterpret_cast< const uint8_t *>( x) - m_Page) /m_ObjSz);
        XD_SANITY_ASSERT( id < Mx)
        return id; 
    } 
};

//_____________________________________________________________________________________________________________________________
// temporary cache at each CPU so that only bulk ops are carried out with SpinLocks.

template < uint32_t CacheSz, typename Store>
struct Cv_FreeCache
{ 
    typedef  typename Store::Type                   Type;
    typedef  typename Store::Stor                   Stor;
    typedef  typename Store::Atomic                 Atomic;
    typedef  typename Cv_Spinlock<Atomic>::Guard    Guard;
    
    Store                       *m_FreeStore;
    Cv_Array< Stor, CacheSz>    m_CacheStore;
//    std::vector< bool>          m_Bits;
    
    Cv_FreeCache( Store *freeStore)
       : m_FreeStore( freeStore)
        //, m_Bits( Store::StorMax, false)
    {} 
    
    Store           *GetStore( void) const { return m_FreeStore; }
    uint8_t         StoreId( void) const { return  m_FreeStore->m_StoreId; }
    uint32_t        SzFree( void)  const { return  m_CacheStore.m_SzFill; }
        
    uint32_t        ProbeSzFree( uint32_t szExpect = 1)                 //  The number of free entries in Cache, if none try fetch.
    { 
        if ( m_CacheStore.m_SzFill >= szExpect)
            return  m_CacheStore.m_SzFill; 
        {
            Guard      guard( &m_FreeStore->m_Lock); 
            m_FreeStore->AllocBulk( &m_CacheStore); 
        }
        return  m_CacheStore.m_SzFill;
    }
    
    uint32_t    AllocBulk( Type **xArr, uint32_t sz)                    // allocate from cache and return pointers to free location
    {        
        uint32_t    szAlloc =  sz < m_CacheStore.SzFill() ? sz : m_CacheStore.SzFill();
        for ( uint32_t i = 0; i < szAlloc; ++i)
        {
            xArr[ i] = m_FreeStore->GetAt( m_CacheStore[ m_CacheStore.SzFill() -szAlloc +i ]);
            //XD_PREFETCH_CACHE( xArr[ i])
            *(( uint64_t *) xArr[ i]) = 0;
        }
        m_CacheStore.m_SzFill -= szAlloc;
        return szAlloc;
    }
     
    
    Type       *AllocFree( void)                                        // return pointer to free location
    { 
        Stor    id = m_CacheStore[ --m_CacheStore.m_SzFill];
//        XD_ERROR_ASSERT( !m_Bits[ id] && (( m_Bits[ id] = true)))
        Type    *x = m_FreeStore->GetAt( id); 
        XD_SANITY_ASSERT( x->SetStoreId( m_FreeStore->m_StoreId))
        return ::new (x) Type(); 
    } 
    
    void    Discard( Stor id)                                           // discard object at Id
    { 
//        XD_ERROR_ASSERT( m_Bits[ id] && (!( m_Bits[ id] = false)))
        if ( m_CacheStore.m_SzFill == m_CacheStore.Size())
        {
            Guard      guard( &m_FreeStore->m_Lock);                    // setup an SpinLock guard and return to buffer to  Store if overflow
            m_FreeStore->DiscardBulk( &m_CacheStore); 
        }
        m_CacheStore[ m_CacheStore.m_SzFill] = id;
        m_CacheStore.m_SzFill += 1;
    }
    
    void    Discard( Type *x) 
    {
        XD_SANITY_ASSERT( x->StoreId() == m_FreeStore->m_StoreId)
        x->Type::~Type();
        //memset( x, 0xCC, Store::ObjSz);
        Discard( m_FreeStore->MapId( x)); 
    }    // Discard an object
}; 

//_____________________________________________________________________________________________________________________________
