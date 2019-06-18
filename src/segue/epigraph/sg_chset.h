// sg_chset.h ___________________________________________________________________________________________________________________
#pragma once


#include    "segue/epigraph/sg_bitset.h"
#include    "segue/epigraph/sg_partition.h"

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
    
    Sg_ChSet( const Sg_Bitset< 256> &bitset)
        : Sg_Bitset< 256>( bitset)
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

struct Sg_Partition : public Sg_CharPartition< 256>
{
    std::string     ToString( void) const
    {
        std::stringstream       strStrm;
        strStrm << "[ ";
        std::vector< Sg_Bitset< 256> >    dom = Domain();
        for ( uint32_t k = 0; k < dom.size(); ++k)
        {
            Sg_ChSet    chSet( dom[ k]);
            strStrm <<  chSet.ToString() << " ";
        }
        strStrm << "]";
        return strStrm.str();
    }
};

//_____________________________________________________________________________________________________________________________
