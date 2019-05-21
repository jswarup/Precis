// cv_array.h ______________________________________________________________________________________________________________
#pragma once


//_____________________________________________________________________________________________________________________________

template < typename X, uint32_t Sz, typename SzType = uint32_t, uint32_t ObjSz = 0>
struct Cv_Array   
{
    static constexpr uint32_t  ObjectSz( void) { return ObjSz ? ObjSz : sizeof( X); } 

    SzType                  m_SzFill;                                                           // number of occupied entries
    uint8_t                 m_MemArr[ ObjectSz() * Sz];

    Cv_Array( void)
        : m_SzFill( 0)
    {
        for ( uint32_t i = 0; i < Sz; ++i)
            ::new (PtrAt( i)) X();
    }
    ~Cv_Array( void) 
    {
        for ( uint32_t i = 0; i < Sz; ++i)
            PtrAt( i)->X::~X();
    }

    uint32_t    Size( void) const { return Sz; }
    uint32_t    SzFill( void) const { return m_SzFill; }             
    uint32_t    SzVoid( void) const { return Sz - m_SzFill; }                                   // number of empty entries
    X           *PtrVoid( void)  { return PtrAt( m_SzFill); }                                      // iterator at the empty slot
    X           *PtrAt( uint32_t k) { return reinterpret_cast< X *>( &m_MemArr[ k *ObjectSz() ]); }
    const X     *PtrAt( uint32_t k) const { return reinterpret_cast< const X *>( &m_MemArr[ k *ObjectSz() ]); }

    X           &At( uint32_t k) { return *reinterpret_cast< X *>( &m_MemArr[ k *ObjectSz() ]); }
    const X     &At( uint32_t k) const { return *reinterpret_cast< const X *>( &m_MemArr[ k *ObjectSz() ]); }

    void        MarkFill( uint32_t sz) { m_SzFill += sz; }                                      // increment additional filled slots

    template < typename Array>     
    void        TransferTo( Array *arr)                                                         // transfer to input array
    {
        uint32_t    szCacheVoid = arr->SzVoid();                                                // space in incoming Array
        uint32_t    szAlloc =  szCacheVoid < m_SzFill ? szCacheVoid : m_SzFill;                 // Qty to be moved.
        std::copy( PtrAt( m_SzFill -szAlloc), PtrAt( m_SzFill), arr->PtrAt( arr->SzFill()));             // copy the element from the end
        m_SzFill -= szAlloc;                                                                    // reduce the SzFill by number transferred
        arr->MarkFill( szAlloc);                                                                // increment the filled amount in input-array
        return;
    } 

    void EraseAt( uint32_t k)                                                                   // erase an occupied element at index
    {
        std::copy( PtrAt( k +1), PtrAt( m_SzFill), PtrAt( k));
        --m_SzFill;
    }

    void        Append( const X &x) { *PtrAt( m_SzFill++) = x; }                                   // apppend one at end

    X           &operator[]( uint32_t i) { return *PtrAt( i); }
    const X     &operator[]( uint32_t i) const { return *PtrAt( i); }

    void        LShift( uint32_t from)
    {
        uint32_t        szBal = m_SzFill - from;
        for ( uint32_t i = 0; i < szBal; ++i)
            SELF[ i] = SELF[ i +from];        
        m_SzFill = szBal;
    }

};

//_____________________________________________________________________________________________________________________________

template < typename X>
class Cv_CArr
{
protected:
    X                   *m_CStr;
    uint32_t            m_Len;

public:  
    Cv_CArr( void)
        : m_CStr( NULL), m_Len( 0)
    {}
    
    Cv_CArr( X *ptr, uint32_t len)
        : m_CStr( ptr), m_Len( len)
    {}

    Cv_CArr( const X *ptr, const X *end)
        : m_CStr( ptr), m_Len( uint32_t( end -ptr))
    {}

    X           *Ptr( void) const { return m_CStr; }
    uint32_t    Size( void) const { return m_Len; }

    X           *Begin( void) const { return Ptr(); }
    X           *End( void) const { return Ptr() +m_Len; }

    const X     &operator[]( uint32_t i) const { return m_CStr[ i]; } 
    X           &operator[]( uint32_t i) { return m_CStr[ i]; }
    const X     &At( uint32_t i) const { return m_CStr[ i]; }  

    void        Advance( uint32_t k)
    {
        if ( k > m_Len)
            k = m_Len;
        m_Len -= k;
        m_CStr = m_Len ? ( m_CStr + k) : NULL;
        return;
    } 
    
    Cv_CArr     Ahead( uint32_t k)
    {
        Cv_CArr     arr = SELF;
        arr.Advance( k);
        return arr;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename X, typename TypeSZ = uint32_t>
struct Cv_LinArr
{
    TypeSZ      m_Sz;

    TypeSZ      Size( void) const { return m_Sz; }

    const X     *Ptr( void) const { return ( &m_Sz +1); }
    X           *Ptr( void) { return ( &m_Sz +1); }

    X           *Begin( void) const { return Ptr(); }
    X           *End( void) const { return Ptr() +m_Sz; }

    const X     &operator[]( uint32_t i) const { return Ptr()[ i]; } 
    X           &operator[]( uint32_t i) { return Ptr()[ i]; } 
    
    static Cv_LinArr   *Construct( TypeSZ sz)
    {
        Cv_LinArr   *arr = ::new ( new char[ sizeof( Cv_LinArr) + sz * sizeof( X)]) Cv_LinArr();
        arr->m_Sz = sz;
        for ( uint32_t i = 0; i < Size(); ++i) 
            ::new (Ptr() +i) X(); 
    }

    struct LessOp
    {
        bool operator()( const X *x1,  const X *x2) const 
        {
            if ( x1->Size() != x2->Size())
                return x1->Size() < x2->Size();
            const X       *arr1 = x1->Ptr();
            const X       *arr2 = x2->Ptr();
            for ( uint32_t i = 0; i < x1->Size(); ++i)
                if ( arr1[ i] != arr2[ i])
                    return arr1[ i] < arr2[ i];
            return false;
        }
    };
};

//_____________________________________________________________________________________________________________________________

