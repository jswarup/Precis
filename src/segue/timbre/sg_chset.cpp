// sg_chset.cpp ___________________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/timbre/sg_chset.h"

//_____________________________________________________________________________________________________________________________


//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::All( void)
{
    static Sg_ChSet   s_CCL = Sg_ChSet().Negative();
    return s_CCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::Digit( void)
{
    static Sg_ChSet   s_CCL( isdigit);
    return s_CCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::NonDigit( void)
{
    static Sg_ChSet   s_CCL = Digit().Negative();
    return s_CCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::Word( void)
{
    static Sg_ChSet   s_WordCCL;
    static bool     fOnce = false;

    if ( fOnce)
        return s_WordCCL;
    fOnce = true;

    // Word Class
    s_WordCCL.Set( '_', true);
    s_WordCCL.SetByteRange( 'a','z',1);
    s_WordCCL.SetByteRange( 'A','Z',1);
    s_WordCCL.SetByteRange( '0','9',1);
    return s_WordCCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::NonWord( void)
{
    static Sg_ChSet   s_NonWordCCL = Word().Negative();
    return s_NonWordCCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::AlphaNum( void)
{
    static Sg_ChSet   s_CCL( isalnum);
    return s_CCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::Ascii( void)
{
    static Sg_ChSet   s_CCL( isascii);
    return s_CCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::Blank( void)
{
    static Sg_ChSet   s_CCL;
    static bool     fOnce = false;

    if ( fOnce)
        return s_CCL;
    fOnce = true;
    s_CCL.Set( ' ', true);
    s_CCL.Set( '\t', true);
    return s_CCL;
}
//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::EndLine( void)
{
    static Sg_ChSet   s_CCL;
    static bool     fOnce = false;

    if ( fOnce)
        return s_CCL;
    fOnce = true;
    s_CCL.Set( '\n', true);
    return s_CCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::Cntrl( void)
{
    static Sg_ChSet   s_CCL( iscntrl);
    return s_CCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::Graph( void)
{
    static Sg_ChSet   s_CCL( isgraph);
    return s_CCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::Print( void)
{
    static Sg_ChSet   s_CCL( isprint);
    return s_CCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::Punct( void)
{
    static Sg_ChSet   s_CCL( ispunct);
    return s_CCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::Space( void)
{
    static Sg_ChSet   s_CCL( isspace);
    return s_CCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::NonSpace( void)
{
    static Sg_ChSet   s_CCL = Space().Negative();
    return s_CCL;
}


//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::Alpha( void)
{
    static Sg_ChSet   s_CCL( isalpha);
    return s_CCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::Upper( void)
{
    static Sg_ChSet   s_CCL( isupper);
    return s_CCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::Lower( void)
{
    static Sg_ChSet   s_CCL( islower);
    return s_CCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::XDigit( void)
{
    static Sg_ChSet   s_CCL( isxdigit);
    return s_CCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::Dot( void)
{
    static Sg_ChSet   s_CCL = DotAll();
    static bool     fOnce = false;

    if ( fOnce)
        return s_CCL;
    fOnce = true; 
    return s_CCL;
}

//_____________________________________________________________________________________________________________________________

const Sg_ChSet    &Sg_ChSet::DotAll( void)
{
    static Sg_ChSet   s_CCL = Sg_ChSet().Negate();
    return s_CCL;
}

//_____________________________________________________________________________________________________________________________
