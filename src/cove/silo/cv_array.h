// cv_array.h ______________________________________________________________________________________________________________
#pragma once

//_____________________________________________________________________________________________________________________________

template < typename X, uint32_t XSize, typename SzType = uint32_t>
struct Cv_Array   
{ 
    enum 
    {
        Sz = XSize
    };

    SzType                  m_SzFill;                                                           // number of occupied entries
    X                       m_Arr[  Sz];

    Cv_Array( void)
        : m_SzFill( 0)
    {}

    ~Cv_Array( void) 
    {}


    uint32_t    Size( void) const { return Sz; }
    uint32_t    SzFill( void) const { return m_SzFill; }             
    uint32_t    SzVoid( void) const { return Sz - m_SzFill; }                                   // number of empty entries
    X           *PtrVoid( void)  { return PtrAt( m_SzFill); }                                      // iterator at the empty slot
    X           *PtrAt( uint32_t k) { return &m_Arr[ k]; }
    const X     *PtrAt( uint32_t k) const { return &m_Arr[ k]; }

    X           &At( uint32_t k) { return m_Arr[ k]; }
    const X     &At( uint32_t k) const { return m_Arr[ k]; }

    void        Reset( void) { m_SzFill = 0; }
    void        Reset( SzType sz, const X &x) 
    { 
        m_SzFill = sz; 
        for ( uint32_t i = 0; i < sz; ++i)
            m_Arr[ i] = x;
    }

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
class Cv_Seq
{
protected:
    X                   *m_Arr;
    uint32_t            m_Len;

public:  
    Cv_Seq( void)
        : m_Arr( NULL), m_Len( 0)
    {}
    
    Cv_Seq( X *ptr, uint32_t len)
        : m_Arr( ptr), m_Len( len)
    {}

    Cv_Seq( const X *ptr, const X *end)
        : m_Arr( ptr), m_Len( uint32_t( end -ptr))
    {}
  
    X           *Ptr( void) const { return m_Arr; }
    uint32_t    Size( void) const { return m_Len; }

    X           *Begin( void) const { return Ptr(); }
    X           *End( void) const { return Ptr() +m_Len; }

    const X     &operator[]( uint32_t i) const { return m_Arr[ i]; } 
    X           &operator[]( uint32_t i) { return m_Arr[ i]; }
    const X     &At( uint32_t i) const { return m_Arr[ i]; }  
    X           *PtrAt( uint32_t i) { return &m_Arr[ i]; }  

    void        Advance( uint32_t k)
    {
        if ( k > m_Len)
            k = m_Len;
        m_Len -= k;
        m_Arr = m_Len ? ( m_Arr + k) : NULL;
        return;
    } 
    
    Cv_Seq     Ahead( uint32_t k) const
    {
        Cv_Seq     arr = SELF;
        arr.Advance( k);
        return arr;
    }
};

//_____________________________________________________________________________________________________________________________
 

template < typename X, typename TypeSZ = uint32_t>
struct Cv_Linear
{
    TypeSZ      m_Sz;

    Cv_Linear( TypeSZ sz)
        : m_Sz( sz)
    {}

    TypeSZ      Size( void) const { return m_Sz; }

    static uint32_t     TrailSz( TypeSZ sz) { return  sz * sizeof( X); }

    static void     Init( void)
    {
        for ( uint32_t i = 0; i < Size(); ++i) 
            ::new (Ptr() +i) X(); 
    }

    const X     *Ptr( void) const { return ( const X *) ( &m_Sz +1); }
    X           *Ptr( void) { return ( X *) ( &m_Sz +1); }

    X           *Begin( void) const { return Ptr(); }
    X           *End( void) const { return Ptr() +m_Sz; }

    const X     &operator[]( uint32_t i) const { return Ptr()[ i]; } 
    X           &operator[]( uint32_t i) { return Ptr()[ i]; } 
    
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

