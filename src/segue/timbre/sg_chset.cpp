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

int Sg_ChSet::ListChars( int *list) const
{
    int     n = 0;
    for ( uint32_t i = 0; i < 4; i++) 
    {
        uint64_t   b = m_ChSet[i];
        for ( uint32_t j = 0; b; j++, b >>= 1) 
        {
            if ( b & 1)
                list[n++] = (i << 6) | j;
        }
    }
    return n;
}

//_____________________________________________________________________________________________________________________________

static     const int   CCLPRETTYPRINT_NUMSTRINGS = 8;
static     const int   CCLPRETTYPRINT_STRINGLEN = 512;

#define CHARPRETTYPRINT_NORMAL 0
#define CHARPRETTYPRINT_CHARCLASS 1

char *charPrettyPrint ( int c, char *p, int mode )
{
	bool escape = false;
	bool hex = false;
	if ( c >= 256 )
	{
		sprintf ( p, "\\x%03X", c );
		p += 5;
		return p;
	}
	switch ( c )
	{
		case '\t' :
			sprintf ( p, "\\t" );
			p += 2;
			return p;
		case '\n' :
			sprintf ( p, "\\n" );
			p += 2;
			return p;
		case '\r' :
			sprintf ( p, "\\r" );
			p += 2;
			return p;
		case '\f' :
			sprintf ( p, "\\f" );
			p += 2;
			return p;
		case '\a' :
			sprintf ( p, "\\a" );
			p += 2;
			return p;
		/*case 0x1b :
			sprintf ( p, "\\e" );
			p += 2;
			return p; */
		case 0x0b :
			sprintf ( p, "\\v" );
			p += 2;
			return p;
	}
	if ( mode == CHARPRETTYPRINT_NORMAL && 1 )
	{
		if ( strchr ( "'\"=", c ) )
			hex = 1;
		if ( strchr ( "^$ *+{}[].\\/|?", c ) )
			escape = 1;
	}
	else if ( mode == CHARPRETTYPRINT_CHARCLASS && 1 )
	{
		if ( strchr ( "^[]\\/-", c ) )
			escape = 1;
	}
	if ( !isalnum ( c ) && ( c != '.') && ( c != '$') && ( c != '@') && ( c != '_') )
		hex = 1;
	if ( hex )
	{
		sprintf ( p, "\\x%02X", c );
		p += 4;
	}
	else if ( escape )
	{
		sprintf ( p, "\\%c", c );
		p += 2;
	}
	else
	{
		*p = c;
		p++;
	}
	*p = '\0';
	return p;
}

//_____________________________________________________________________________________________________________________________

std::string     Sg_ChSet::ToString ( void) const
{


    if ( IsEqual( Word()))
        return "[[Word]]";
    if ( IsEqual( NonWord()))
        return "[[NonWord]]";

	static int  stringIdx = 0;
	static char strings[ CCLPRETTYPRINT_NUMSTRINGS][ CCLPRETTYPRINT_STRINGLEN];
	int         posChars[ 259], negChars[ 259];
	char        *str = strings[ stringIdx];
    
	stringIdx = ( stringIdx + 1 ) % CCLPRETTYPRINT_NUMSTRINGS;

	char        negStr[ CCLPRETTYPRINT_STRINGLEN] = ""; 
    char        posStr[ CCLPRETTYPRINT_STRINGLEN] = "";
	int         posWeight = ListChars ( posChars );
	int         eos = Get ( 256);
	int         eop = Get ( 257);
	Sg_ChSet      negCcl = Negative();
    negCcl.m_ChSet[ 32] = 0;  // No EOP,EOS in inverse
	int             negWeight = negCcl.ListChars( negChars);
	for ( int neg = 0; neg < 2; neg++ )
	{
		if ( neg && ( eop || eos ) )
		{
			break;
		}
		Sg_ChSet    curCcl ( neg ? negCcl : SELF );
		char            *curStr = ( neg ? negStr : posStr );
		int             *curChars = ( neg ? negChars : posChars );
		int             curWeight = ( neg ? negWeight : posWeight );
		char            *p = curStr;
		*( p++ ) = '[';
		if ( neg )
		{
			* ( p++ ) = '^';
		}
		for ( int i = 0; i < curWeight;  )
		{
            int     j = i + 1;
			for ( ; j < curWeight; j++ )
			{
				if ( curChars[j] != curChars[j-1] + 1 )
					break;
			}
			int     len = j - i;
			p = charPrettyPrint ( curChars[i], p, CHARPRETTYPRINT_CHARCLASS );
			if ( len > 2 )
				* ( p++ ) = '-';
			if ( len > 1 )
				p = charPrettyPrint ( curChars[i+len-1], p, CHARPRETTYPRINT_CHARCLASS );
            i += len;
		}
		* ( p++ ) = ']';
		*p = '\0';
	}
	if ( eop || eos || strlen ( posStr ) <= strlen ( negStr ) || negWeight == 0 )
		strcpy ( str, posStr );
	else
		strcpy ( str, negStr );
	return str;
}

//_____________________________________________________________________________________________________________________________
