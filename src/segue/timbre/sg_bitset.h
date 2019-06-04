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
        std::fill_n( &m_ChSet[ 0], Sz, 0); 
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

    int32_t         Compare( const Sg_Bit64 &chSet) const  
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
    
    bool            IsOnes( void) const 
    {
        for ( uint32_t i = 0; i < Sz; ++i)
            if ( m_ChSet[ i] != CV_UINT64_MAX)
                return false;
        return true;
    }

template < typename Lambda, typename... Args>
    void    ForAllTrue( const Lambda &lambda,  const Args&... args)  
    {
        for ( uint32_t i = 0; i < Sz; i++) 
        {
            uint64_t   b = m_ChSet[ i];
            for ( uint32_t j = 0; b; j++, b >>= 1)  
                if ( b & 1)
                    lambda( (i << 6) | j, args...); 
        }
        return;
    }

    uint32_t         Index( bool t) const
    {
        for ( uint32_t i = 0; i < Sz; i++) 
        {
            uint64_t   b = t ? m_ChSet[ i] : ~m_ChSet[ i];
            for ( uint32_t j = 0; b; j++, b >>= 1)  
                if ( b & 1)
                    return (i << 6) | j; 
        }
        return CV_UINT32_MAX;
    }

    uint8_t         PopCount( void)
    {
        uint8_t     cnt = 0;
        for ( uint32_t i = 0; i < Sz; i++) 
            cnt +=  cv_PopCount( m_ChSet[ i]);
        return cnt;
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
    bool            IsOnes( void) const { return true; }

template < typename Lambda, typename... Args>
    void            ForAllTrue( const Lambda &lambda,  const Args&... args)   {}

    uint32_t        Index( bool t) const { return CV_UINT32_MAX; }
    uint8_t         PopCount( void) { return 0; }
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
    
    bool            IsOnes( void) const 
    {
        for ( uint32_t i = 0; i < Sz; ++i)
            if ( m_ChSet[ i] != CV_UINT8_MAX)
                return false;
        return true;
    }
    
template < typename Lambda, typename... Args>
    void            ForAllTrue( const Lambda &lambda,  const Args &... args)
    {
        for ( uint32_t i = 0; i < Sz; i++) 
        {
            uint64_t   b = m_ChSet[ i];
            for ( uint32_t j = 0; b; j++, b >>= 1)  
            if ( b & 1)
                lambda( (i << 3) | j, args...);
        } 
        return;
    }

    uint32_t         Index( bool t) const
    {
        for ( uint32_t i = 0; i < Sz; i++) 
        {
            uint64_t   b = t ? m_ChSet[ i] : ~m_ChSet[ i];
            for ( uint32_t j = 0; b; j++, b >>= 1)  
                if ( b & 1)
                    return (i << 3) | j; 
        }
        return CV_UINT32_MAX;
    }

    uint8_t         PopCount( void)
    {
        uint8_t     cnt = 0;
        for ( uint32_t i = 0; i < Sz; i++) 
            cnt += cv_PopCount( m_ChSet[ i]);
        return cnt;
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
    bool            IsOnes( void) const { return true; }

 template < typename Lambda, typename... Args>
    void            ForAllTrue( const Lambda &lambda,  const Args&... args)   {}
    uint32_t        Index( bool t) const { return CV_UINT32_MAX; }
    uint8_t         PopCount( void) { return 0; }
};

//_____________________________________________________________________________________________________________________________

template < uint32_t SzBits>
struct Sg_Bitset
{
    enum {
        Sz64 = SzBits/64,
        Width64 = Sz64 * 64,
        Sz8 =  (( SzBits - Width64) + 7)/8,
        SzChBits = SzBits,
    };
    
    Sg_Bit64< Sz64>     m_Bits64;
    Sg_Bit8< Sz8>       m_Bits8;

    typedef Sg_Bit64< Sz64> Bit64;
    typedef Sg_Bit64< Sz8>  Bit8;

    Sg_Bitset( void) { Init(); }
    Sg_Bitset( const Sg_Bitset &bitset)
        : m_Bits64( bitset.m_Bits64), m_Bits8( bitset.m_Bits8)
    {}

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
    
    Sg_Bitset       Negative( void) const { Sg_Bitset   neg( SELF); neg.Negate(); return neg; }

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
        int32_t     n = m_Bits64.ListChars( list);
        return  n + m_Bits8.ListChars( list +n); 
    }

    bool            IsOnes( void) const  { return m_Bits64.IsOnes() && m_Bits64.IsOnes();  }

template < typename Lambda, typename... Args>
    void            ForAllTrue( const Lambda &lambda,  const Args&... args)   
    {
        m_Bits64.ForAllTrue( lambda, args...);
        m_Bits8.ForAllTrue( lambda, args...);
    }
    
    uint32_t         Index( bool t)
    {
        uint32_t    ind = m_Bits64.Index( t);
        return ( ind != CV_UINT32_MAX) ? ind : m_Bits8.Index( t);
    }

    void    SetFilter(  int (*filter)( int c))
    {
        for ( uint32_t i = 0; i < SzChBits; ++i) 
            Set( i, filter( i));
    }

    uint32_t         RepIndex( void)
    {
        for ( uint32_t i = 0; i < SzBits; ++i)
            if ( Get( i))
                return i;
        return CV_UINT32_MAX;
    }

    uint8_t         PopCount( void) 
    { 
        return m_Bits64.PopCount() +m_Bits8.PopCount();        
    }

    std::string     ToString( void) const
    {
        std::stringstream   sstr;
        sstr << '['; 
        int         curChars[ 256];
        int         curWeight = ListChars( curChars);   

        for ( int i = 0; i < curWeight;  )
        {
            int     j = i + 1;
            for ( ; j < curWeight; j++ )
            {
                if ( curChars[j] != curChars[j-1] + 1 )
                    break;
            }
            int     len = j - i;
            sstr << uint32_t( curChars[i]);
            if ( len > 2 )
                sstr<< '-';
            if ( len > 1 )
                sstr << uint32_t( curChars[ i +len -1]);
            i += len;
        } 
        sstr << ']';
        return sstr.str();
    }
};

//_____________________________________________________________________________________________________________________________
