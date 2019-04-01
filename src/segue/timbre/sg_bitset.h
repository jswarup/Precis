// sg_bitset.h ___________________________________________________________________________________________________________________
#pragma once

//_____________________________________________________________________________________________________________________________

template < uint32_t Sz>
struct Sg_Bit64 
{
    enum {
        SzBits = 64,
    };
  
    uint64_t   m_ChSet[ Sz]; 

    void            Init(  void) 
    {
        for ( uint32_t i = 0;  i < Sz; ++i)
            m_ChSet[ i] = 0;
    }

    bool            GetChar( uint32_t c) const  { return ( m_ChSet[ c / SzBits] & (uint64_t( 1) << (c % SzBits))) != 0; }

    void            SetChar( uint32_t c) { m_ChSet[ ((c) / SzBits) ] |= ( uint64_t( 1) << ((c) % SzBits)); } 
    void            ClearChar( uint32_t c)  { m_ChSet[ ((c) / SzBits) ] &= ~(uint64_t( 1) << ((c) %  SzBits)); }
    
    void            Negate( void)
    { 
        for ( uint32_t i = 0; i < Sz; ++i)
            m_ChSet[ i] = ~m_ChSet[ i]; 
    }

    bool            IsIntersect( const Sg_Bit64 &chSet) const
    {
        for ( uint32_t i = 0; i < Sz; ++i)
            if ( m_ChSet[ i] & chSet.m_ChSet[ i])
                return true;
        return false;
    }

    void            UnionWith( const Sg_Bit64 &src2)
    {
        for ( uint32_t i = 0; i < Sz; ++i)
            m_ChSet[ i] |= src2.m_ChSet[ i];
    }

    int32_t     Compare( const Sg_Bit64 &chSet) const  
    {
        for ( uint32_t i = 0; i < Sz; ++i)
            if ( m_ChSet[ i] != chSet.m_ChSet[ i])
                return ( m_ChSet[ i] < chSet.m_ChSet[ i]) ? 1 : -1;
        return 0;
    }
    
    int             ListChars( int *list) const
    {
        int     n = 0;
        for ( uint32_t i = 0; i < Sz; i++) 
        {
            uint64_t   b = m_ChSet[ i];
            for ( uint32_t j = 0; b; j++, b >>= 1) 
            {
                if ( b & 1)
                    list[ n++] = (i << 6) | j;
            }
        }
        return n;
    }
};

template <>
struct Sg_Bit64< 0>
{
    void            Init(  void) { ;  }

    bool            GetChar( uint32_t c) const  { return false; }

    void            SetChar( uint32_t c) { ; } 
    void            ClearChar( uint32_t c)  { ; }
    void            Negate( void) { ; }
    bool            IsIntersect( const Sg_Bit64 &chSet) const { return false; }
    void            UnionWith( const Sg_Bit64 &src2) {}
    int32_t         Compare( const Sg_Bit64 &cs) const   { return 0; }
    int             ListChars( int *list) const   { return 0; }
};

template < uint32_t Sz>
struct Sg_Bit8 
{
    enum {
        SzBits = 8,
    };

    uint8_t   m_ChSet[ Sz]; 

    void            Init(  void) 
    {
        for ( uint32_t i = 0; i < Sz; ++i)
            m_ChSet[ i] = 0;
    }

    bool            GetChar( uint32_t c) const  { return ( m_ChSet[ c / SzBits] & (uint8_t( 1) << (c % SzBits))) != 0; }

    void            SetChar( uint32_t c) { m_ChSet[ ((c) / SzBits) ] |= ( uint8_t( 1) << ((c) % SzBits)); } 
    void            ClearChar( uint32_t c)  { m_ChSet[ ((c) / SzBits) ] &= ~(uint8_t( 1) << ((c) %  SzBits)); }

    void            Negate( void)
    { 
        for ( uint32_t i = 0; i < Sz; ++i)
            m_ChSet[ i] = ~m_ChSet[ i]; 
    }

    bool            IsIntersect( const Sg_Bit8 &chSet) const
    {
        for ( uint32_t i = 0; i < Sz; ++i)
            if ( m_ChSet[ i] & chSet.m_ChSet[ i])
                return true;
        return false;
    }

    void            UnionWith( const Sg_Bit8 &chSet)
    {
        for ( uint32_t i = 0; i < Sz; ++i)
            m_ChSet[ i] |= chSet.m_ChSet[ i];
    }

    int32_t         Compare( const Sg_Bit8 &chSet) const  
    {
        for ( uint32_t i = 0; i < Sz; ++i)
            if ( m_ChSet[ i] != chSet.m_ChSet[ i])
                return ( m_ChSet[ i] < chSet.m_ChSet[ i]) ? 1 : -1;
        return 0;
    }
     
    int             ListChars( int *list) const
    {
        int     n = 0;
        for ( uint32_t i = 0; i < Sz; i++) 
        {
            uint64_t   b = m_ChSet[ i];
            for ( uint32_t j = 0; b; j++, b >>= 1) 
            {
                if ( b & 1)
                    list[ n++] = (i << 3) | j;
            }
        }
        return n;
    }
};

template <>
struct Sg_Bit8< 0>
{

    void            Init(  void) { ;  }

    bool            GetChar( uint32_t c) const  { return false; }

    void            SetChar( uint32_t c) { ; } 
    void            ClearChar( uint32_t c)  { ; }
    void            Negate( void) { ; }
    bool            IsIntersect( const Sg_Bit8 &chSet) const { return false; }
    void            UnionWith( const Sg_Bit8 &src2) {}
    int32_t         Compare( const Sg_Bit8 &cs) const   { return 0; }
    int             ListChars( int *list) const   { return 0; }
};

//_____________________________________________________________________________________________________________________________

template < uint32_t SzBits>
struct Sg_Bitset
{
    enum {
        Sz64 = SzBits/64,
        Sz8 =  (( SzBits - Sz64 * 8) + 7)/8,
        Width64 = Sz64 * 64,
        SzChBits = SzBits,
    };
    
    Sg_Bit64< Sz64>     m_Bits64;
    Sg_Bit8< Sz8>       m_Bits8;

    typedef Sg_Bit64< Sz64> Bit64;
    typedef Sg_Bit64< Sz8>  Bit8;

    Sg_Bitset( void) { Init(); }

    void            Init(  void) { m_Bits64.Init(); m_Bits8.Init();  }

    bool            IsBit64( uint32_t c ) const { return c < Width64; }

    bool            Get( uint32_t c) const  { return IsBit64( c) ? m_Bits64.GetChar( c) :  m_Bits8.GetChar( c -Width64);  }
 
    void            Set( uint32_t c, bool v)  {  ( v ) ? SetChar( c) : ClearChar( c); }


    void            SetChar( uint32_t c) { IsBit64( c) ? m_Bits64.SetChar( c) : m_Bits8.SetChar( c -Width64); } 
    void            ClearChar( uint32_t c)  { IsBit64( c) ? m_Bits64.ClearChar( c) : m_Bits8.ClearChar( c -Width64); }

    void        Negate( void)
    { 
        m_Bits64.Negate();
        m_Bits8.Negate(); 
    }

    bool        IsIntersect( const Sg_Bitset &chSet) const  { return m_Bits64.IsIntersect( chSet.m_Bits64) || m_Bits8.IsIntersect( chSet.m_Bits8) ; }
    void        UnionWith( const Sg_Bitset &chSet) { m_Bits64.UnionWith( chSet.m_Bits64);  m_Bits8.UnionWith( chSet.m_Bits8); }
    int32_t     Compare( const Sg_Bitset &chSet) const 
    { 
        int32_t  res = m_Bits64.Compare( chSet.m_Bits64);  
        return ( res != 0) ? res : m_Bits8.Compare( chSet.m_Bits8); 
    }

    void        SetByteRange( uint32_t start, uint32_t stop, bool value)
    {
        for ( uint32_t i = start; i <= stop; ++i)
            Set( i, value); 
    }


    int         ListChars( int *list) const   
    { 
        int32_t  n = m_Bits64.ListChars( list);
        return n + m_Bits8.ListChars( list +n); 
    }

    void    SetFilter(  int (*filter)( int c))
    {
        for ( uint32_t i = 0; i < SzChBits; ++i) 
            Set( i, filter( i));
    }
};

//_____________________________________________________________________________________________________________________________
