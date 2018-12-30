// sg_rexpgrammar.h ___________________________________________________________________________________________________________________
#pragma once


#include    "segue/timbre/sg_timbreparser.h"
#include 	"segue/grammar/sg_xmlshard.h" 

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

struct      RExpDocSynElem : public SynElem 
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

	RExpEntry( void)
	{
	}
	 

	auto           Blank( void) const { return CharSet(" \t\v\r\n"); }
	auto           WhiteChars( void) const { return CharSet(" \t\v\r"); }
	auto           WhiteSpace( void) const { return +WhiteChars(); }
	auto           OptWhiteSpace( void) const { return *WhiteChars(); }
	auto           NL( void) const { return !(Char('\r')) >> Char('\n'); }
	auto           OptBlankSpace( void) const { return *(WhiteChars() | NL()); }
	auto           BlankLine( void) const { return OptWhiteSpace() >> (NL() | EoS()); }
	auto           Comment( void) const { return Str( "<!--") >> *( Any() - Str( "-->")) >> Str( "-->"); } 
	 

template < typename Forge>
	bool    DoParse( Forge *ctxt) const
	{    
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

	RExpDoc( void) 
	{}

	auto           DocumentOver( void) const { return []( auto ctxt) {  
		std::cout << ctxt.MatchStr() << "\n";
		return true;  };  } 

	auto           PI( void) const { return Str( "<?") >> *( Any() - Str( "?>")) >> Str( "?>"); } 

	auto           Document( void) const { return +( RExpEntry() | PI() )[ DocumentOver()]; } 

template < typename Forge>
	bool    DoParse( Forge *ctxt) const
	{   
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
