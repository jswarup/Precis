// sg_rexpgrammar.h ___________________________________________________________________________________________________________________
#pragma once


#include    "segue/timbre/sg_timbreparser.h"
#include 	"segue/grammar/sg_xmlshard.h" 
#include 	"segue/timbre/sg_parsenumerics.h" 

//_____________________________________________________________________________________________________________________________

namespace Sg_RExp
{
using namespace Sg_Timbre; 

//_____________________________________________________________________________________________________________________________

struct      RExpSynElem : public SynElem 
{ 
	Cv_CrateId		m_Item;

	std::string		GetName( void) const { return Cv_Aid::ToStr( "XMLElem", GetId()); } 

	bool    WriteDot( Cv_DotStream &strm)  
	{
		strm << "R" << m_IPtr << " [ shape=diamond  label= <<FONT> #" << GetName() << " <BR />"; 
		strm << " </FONT>>];\n "; 
		return true;
	} 
};


//_____________________________________________________________________________________________________________________________

struct      RExpDocSynElem : public AltSynElem
{ 
	Cv_CrateId		m_Item;

	std::string		GetName( void) const { return Cv_Aid::ToStr( "XDocElem", GetId()); } 

	bool    WriteDot( Cv_DotStream &strm)  
	{
		strm << "R" << m_IPtr << " [ shape=diamond  label= <<FONT> #" << GetName() << " <BR />"; 
		strm << " </FONT>>];\n "; 
		return true;
	}

};  
 

//_____________________________________________________________________________________________________________________________

struct RExpEntry : public Shard< RExpEntry>
{
	struct Whorl
	{
		uint64_t	m_Index;
		SeqSynElem	m_Rules;

		Whorl( void)
			: m_Index( 0)
		{}
	};
	Whorl		*m_CurWhorl;

	RExpEntry( void)
		: m_CurWhorl( NULL)
	{}
	 
	auto           RExpressionListener(void) const {
		return [](auto ctxt) {
			std::cout << ctxt.MatchStr() << "\n";
			return true;  };
	}

	auto          IndexListener(void) const {
		return [this](auto ctxt) {   
			Whorl	*whorl = ctxt.GetParser()->TopWhorl< RExpEntry>();
			whorl->m_Index = ctxt.num; 
			return true;  };
	}

	auto           Blank( void) const { return CharSet(" \t\v\r\n"); }
	auto           WhiteChars( void) const { return CharSet(" \t\v\r"); }
	auto           WhiteSpace( void) const { return +WhiteChars(); }
	auto           OptWhiteSpace( void) const { return *WhiteChars(); }
	auto           NL( void) const { return !(Char('\r')) >> Char('\n'); }
	auto           OptBlankSpace( void) const { return *(WhiteChars() | NL()); }
	auto           BlankLine( void) const { return OptWhiteSpace() >> (NL() | EoS()); }
	auto           Comment( void) const { return Str( "<!--") >> *( Any() - Str( "-->")) >> Str( "-->"); } 
	
	auto           RExpression(void) const { return (+( Any() - Char('/')))[ RExpressionListener()]; }
	auto           RExpEnd( void) const { return Char( '/')  ; }
	auto           RExpBegin( void) const { return ( Char( '/') >>  OptBlankSpace()); }
	auto		   RExpLine(void) const { return  ( Comment()  | ( OptBlankSpace()  >> ParseInt<uint64_t>()[IndexListener()] >> Char(',') >> RExpBegin() >> RExpression() >> RExpEnd() >> BlankLine())); }

template < typename Forge>
	bool    DoParse( Forge *ctxt) const
	{     
		ctxt->Push();
		auto	rexpLine = RExpLine( ); 
		if (!rexpLine.DoMatch(ctxt))
			return false;
		std::cout << ctxt->m_Index << "\n";
		return true;
	}
	 
template < typename Cnstr>
	auto        FetchElemId( Cnstr *cnstr)
	{  
		auto	elem = new RExpSynElem();  
		return cnstr->Store( elem);
	}
};

//_____________________________________________________________________________________________________________________________

struct RExpDoc  : public Shard< RExpDoc>
{  
	struct Whorl
	{
		AltSynElem	m_AllRules;
	};

	RExpDoc( void) 
	{}

	auto           DocumentOver( void) const { return []( auto ctxt) {  
		std::cout << ctxt.MatchStr() << "\n";
		return true;  };  } 
	 
	auto           Document( void) const { return (+RExpEntry())[ DocumentOver()]; } 

template < typename Forge>
	bool    DoParse( Forge *ctxt) const
	{   
		ctxt->Push();
		auto    doc = Document();
		if (  !doc.DoMatch( ctxt))
			return false;  
		return true;
	} 

template < typename Cnstr>
	auto        FetchElemId( Cnstr *cnstr)
	{  
		auto	elem = new RExpDocSynElem();  
		auto	node = Document();
		elem->m_Item = cnstr->FetchElemId( &node);
		return cnstr->Store( elem);
	} 
};
//_____________________________________________________________________________________________________________________________ 

typedef Cv_Crate< RExpDocSynElem, RExpSynElem,  SynParserCrate>   RExpParserCrate; 

//_____________________________________________________________________________________________________________________________
};
