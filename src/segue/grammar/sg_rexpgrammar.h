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

struct RExpPrimitive
{
	auto           Blank(void) const { return CharSet(" \t\v\r\n"); }
	auto           WhiteChars(void) const { return CharSet(" \t\v\r"); }
	auto           WhiteSpace(void) const { return +WhiteChars(); }
	auto           OptWhiteSpace(void) const { return *WhiteChars(); }
	auto           NL(void) const { return !(Char('\r')) >> Char('\n'); }
	auto           OptBlankSpace(void) const { return *(WhiteChars() | NL()); }
	auto           BlankLine(void) const { return OptWhiteSpace() >> (NL() | EoS()); }
	auto           Comment(void) const { return Str("<!--") >> *(Any() - Str("-->")) >> Str("-->"); }
	auto		   AlphaNum(void) const { return CharSet("a-zA-Z0-9"); }
};

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

struct RExpUnit : public Shard< RExpUnit>, public RExpPrimitive
{
	struct Whorl
	{
		Sg_ChSet		m_ChSet;

		Whorl(void)
		{}
		~Whorl(void)
		{}
	}; 

	static constexpr const char   *EscapedCharset = ".^$*+?()[{\\|";    // "[]+*?{}().\\/"

	auto		CharListener(void) const {
		return [](auto ctxt) {
			Whorl	*whorl = ctxt->Pred< RExpUnit>();
			whorl->m_ChSet.Set( ctxt->MatchStr()[0], true);
			return true;  };
	}
	auto		CtrlCharListener(void) const {
		return [](auto ctxt) {
			Whorl	*whorl = ctxt->Pred< RExpUnit>();
			whorl->m_ChSet.Set( ctxt->MatchStr()[0] -'a', true);
			return true;  };
	}
	auto		EscCharListener(void) const {
		return [](auto ctxt) {
			Whorl	*whorl = ctxt->Pred< RExpUnit>();
			whorl->m_ChSet.Set( ctxt->MatchStr()[0], true);
			return true;  };
	}
	auto		OctalListener(void) const {
		return [](auto ctxt) {
			std::cout << ctxt->MatchStr() << "\n";
			return true;  };
	}
	auto		BackrefListener(void) const {
		return [](auto ctxt) {
			std::cout << ctxt->MatchStr() << "\n";
			return true;  };
	}
	auto		HexListener(void) const {
		return [](auto ctxt) {
			std::cout << ctxt->MatchStr() << "\n";
			return true;  };
	}

	auto           Unit(void) const { return AlphaNum()[CharListener()] | 
											 ( Char('\\') >> ( CharSet("abfnrtv")[ CtrlCharListener()] |
												  CharSet( EscapedCharset )[ EscCharListener()]  | 
	 												ParseInt< uint8_t, 8, 2, 3>()[ OctalListener()] |
													ParseInt< uint16_t, 10, 1, 3>()[ BackrefListener()] |
													(IStr( "x") >> ParseInt< uint8_t, 16, 0, 2>()[ HexListener()]))); }

template < typename Forge>
	bool    DoParse( Forge *ctxt) const
	{
		ctxt->Push();
		auto	unit = Unit();
		if ( !unit.DoMatch( ctxt))
			return false; 
		return true;
	}
};

//_____________________________________________________________________________________________________________________________

struct RExpEntry : public Shard< RExpEntry>, public RExpPrimitive
{
	struct Whorl
	{
		uint64_t					m_Index;
		std::vector< Sg_ChSet>		m_ChSets;

		Whorl( void)
			: m_Index( 0)
		{}

		~Whorl(void)
		{}
	};

	RExpEntry( void) 
	{} 

	auto           RExpressionListener(void) const {
		return []( auto ctxt) {
			std::cout << ctxt->MatchStr() << "\n";
			return true;  };
	}

	auto          IndexListener(void) const {
		return [this]( auto ctxt) {   
			Whorl	*whorl = ctxt->Pred< RExpEntry>();
			whorl->m_Index = ctxt->num;
			return true;  };
	}

	auto          UnitListener(void) const {
		return [this]( auto ctxt) {
			Whorl	*whorl = ctxt->Pred< RExpEntry>();
			whorl->m_ChSets.push_back( ctxt->m_ChSet);
			return true;  };
	}

	auto           RExpression(void) const { return (+(RExpUnit()[ UnitListener()] - Char('/')))[ RExpressionListener()]; }
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
		std::cout << ctxt->MatchStr() << "\n";
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
