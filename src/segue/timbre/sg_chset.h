// sg_chset.h ___________________________________________________________________________________________________________________
#pragma once


#include    "segue/timbre/sg_bitset.h"
//_____________________________________________________________________________________________________________________________

class Sg_ChSet : public Sg_Bitset< 256>
{
public:
    enum 
    { 
        Sz = 4,
    };

protected: 
    
    Sg_ChSet( int (*filter)( int c))
    {
        for ( uint32_t i = 0; i < SzChBits; ++i)
            Set( i, filter( i));
    }

public:
    Sg_ChSet( void)
    {}

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
 

    Sg_ChSet        Union( const Sg_ChSet &src2) const { Sg_ChSet  cpy( SELF); cpy.UnionWith( src2); return cpy; } 
    Sg_ChSet        Negative( void) const { Sg_ChSet cpy( SELF); cpy.Negate(); return cpy;} 
  

    std::string     ToString( bool negFlg) const;
    std::string     ToString( void) const;
};
 

//_____________________________________________________________________________________________________________________________
