// sg_parsenumerics.h ___________________________________________________________________________________________________________________
#pragma once


#include    "segue/timbre/sg_timbreparser.h"

//_____________________________________________________________________________________________________________________________

namespace Sg_Timbre
{

//_____________________________________________________________________________________________________________________________

/// Extract optional sign prefix

struct ParseSign
{
template < typename Parser>    
    int     operator()( Parser * parser)
    { 
        int     r = 1;
        switch ( parser->IsCurValid() && parser->Curr())
        {
            case '-':
                r = - 1;
            case '+':
                do
                {
                    parser->Next();
                }
                while ( parser->IsCurValid() && (( parser->Curr() == ' ') || ( parser->Curr() == '\t')));
        }
        return r;
    }
};

//_____________________________________________________________________________________________________________________________

/// Check radix: return 0 <= n < R, or a negative value if Error

template <const int R>
struct ParseDigit
{
template < typename Parser>  
    int     operator()( Parser * parser)
    {
        if ( !parser->IsCurValid())
            return -1;
        int d = parser->Curr() - '0';
        if (d >= R)
            return -1;
        return d;
    }
};

//_____________________________________________________________________________________________________________________________


template <>
struct ParseDigit< 16>
{
template < typename Parser>  
	int operator()(Parser * parser)
    {
        if ( !parser->IsCurValid())
            return -1;
        char    c = parser->Curr();

        if (c < '0') 
            return -1;
        if (c <= '9') 
            return (c - '0');
        if (c < 'A') 
            return -1;
        if (c <= 'F') 
            return (10 + c - 'A');
        if (c < 'a') 
            return -1;
        if (c <= 'f') 
            return (10 + c - 'a');

        return -1;
    }
};

//_____________________________________________________________________________________________________________________________

/// Generic integer parser

template <typename Int = uint64_t, const int Radix = 10, const int MinDigits = 1, const int MaxDigits = 200>
struct ParseInt : public Shard< ParseInt< Int, Radix, MinDigits, MaxDigits> >
{
	struct	Whorl
	{
		Int             num;
		Whorl( void)
			: num( 0)
		{}
	};

template < typename Forge>    
	bool    DoParse( Forge *ctxt) const
	{
		typename Forge::Parser      *parser = ctxt->GetParser();
		
		bool            match = true; 
        int             digit_nb = 0;
		Int				&num = ctxt->num;
        // Check mandatory digits
        while (digit_nb < MinDigits)
        {
            int     d = ParseDigit< Radix>()( parser);
            match = (d >= 0); 
            if ( !match)
                return false;
            
			num = num * Radix + d;
            ++digit_nb;
            parser->Next();
        }  
        // Check optional digits
        while ( match && ( digit_nb < MaxDigits))
        {
            int     d = ParseDigit< Radix>()( parser);
            if ( d < 0)
                break;
			num = num * Radix + d;
            ++digit_nb; 
            parser->Next();
        } 
        return true;
    }
};

//_____________________________________________________________________________________________________________________________

/// No wide string real number parsing implemented

struct ParseReal : public Shard< ParseReal>
{

template < typename Forge>    
	bool    DoParse( Forge *ctxt) const
	{
		typename Forge::TParser      *parser = ctxt->GetParser();


        bool        match = false; 
        double      d = parser->IStrm()->DoubleNext( &match);
		 
        return match;
    }
    
};

//_____________________________________________________________________________________________________________________________
};
