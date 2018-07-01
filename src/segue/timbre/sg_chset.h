// sg_chset.h ___________________________________________________________________________________________________________________
#pragma once

//_____________________________________________________________________________________________________________________________

class Sg_ChSet
{
    enum 
    { 
        Sz = 4,
        SzChBits = 256,
    };

    uint64_t  m_ChSet[4]; 

    void        Init( void) { memset( m_ChSet, 0, sizeof( m_ChSet)); }
    
    Sg_ChSet(  int (*filter)( int c))
    {
        for ( uint32_t i = 0; i < SzChBits; ++i)
            if ( filter( i))
                Set( i, true);
    }

public:
    static const Sg_ChSet    &All( void); 
    static const Sg_ChSet    &Digit( void);
    static const Sg_ChSet    &NonDigit( void);
    static const Sg_ChSet    &Word( void);
    static const Sg_ChSet    &NonWord( void); 
    static const Sg_ChSet    &Space( void);
    static const Sg_ChSet    &NonSpace( void);
    static const Sg_ChSet    &AlphaNum( void);
    static const Sg_ChSet    &Alpha( void);
    static const Sg_ChSet    &Ascii( void);
    static const Sg_ChSet    &Blank( void);
    static const Sg_ChSet    &EndLine( void); 
    static const Sg_ChSet    &Cntrl( void);
    static const Sg_ChSet    &Graph( void);
    static const Sg_ChSet    &Print( void);
    static const Sg_ChSet    &Punct( void);
    static const Sg_ChSet    &XDigit( void);
    static const Sg_ChSet    &Upper( void);
    static const Sg_ChSet    &Lower( void);
    static const Sg_ChSet    &Dot( void);
    static const Sg_ChSet    &DotAll( void); 

    //_________________________________________________________________________________________________________________________

    Sg_ChSet( void) { Init(); }

    bool            Get( uint32_t c) const  { return ( m_ChSet[ c / 64] & (uint64_t( 1) << (c % 64))) != 0; }
    Sg_ChSet        &Set( uint32_t c, bool v) 
    { 
        ( v ) ? m_ChSet[ ((c) / 64) ] |= ( uint64_t( 1) << ((c) % 64)) 
            : m_ChSet[ ((c) / 64) ] &= ~(uint64_t( 1) << ((c) % 8)); 
        return *this; 
    }
 
    Sg_ChSet        &Negate( void)
    { 
        m_ChSet[ 0] = ~m_ChSet[ 0];
        m_ChSet[ 1] = ~m_ChSet[ 1];
        m_ChSet[ 2] = ~m_ChSet[ 2];
        m_ChSet[ 3] = ~m_ChSet[ 3]; 
        return *this;
    }

    Sg_ChSet        Negative( void) const { return Sg_ChSet( SELF).Negate();} 
 
    void            SetByteRange( int start, int stop, bool value); // Start and Stop are inclusive
};
 

//_____________________________________________________________________________________________________________________________
