// sg_xmlgrammar.h ___________________________________________________________________________________________________________________
#pragma once


#include    "segue/timbre/sg_timbreparser.h"
#include 	"segue/grammar/sg_xmlshard.h" 

namespace Sg_Xml
{

using namespace Sg_Timbre; 

//_____________________________________________________________________________________________________________________________

struct      XMLSynElem : public SynElem 
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

struct      XDocSynElem : public SynElem 
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

struct XMLElement : public Shard< XMLElement>
{

    XMLElement( void)
    {
    }

    auto           ElementNameListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt->MatchStr() << "\n";
                                                        return true;  };  }

    auto           ElementLeafOver( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt->MatchStr() << "\n";
                                                        return true;  };  }

    auto           ElementBranchOver( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt->MatchStr() << "\n";
                                                        return true;  };  }

    auto           ElementBranchBegin( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt->MatchStr() << "\n";
                                                        return true;  };  }

    auto           AttributeNameListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt->MatchStr() << "\n";
                                                        return true;  };  }
    
    auto           StringListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt->MatchStr() << "\n";
                                                        return true;  };  }

    auto           QuotListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt->MatchStr() << "\n";
                                                        return true;  };  }

    
    auto           AttributeListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt->MatchStr() << "\n";
                                                        return true;  };  }

    auto           AposListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt->MatchStr() << "\n";
                                                        return true;  };  }

    auto           AmpListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt->MatchStr() << "\n";
                                                        return true;  };  }

    auto           LtListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt->MatchStr() << "\n";
                                                        return true;  };  }

    auto           GtListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt->MatchStr() << "\n";
                                                        return true;  };  }


    auto           DataOver( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt->MatchStr() << "\n";
                                                        return true;  };  }
 

    auto           Blank( void) const { return CharSet(" \t\v\r\n"); }
    auto           WhiteChars( void) const { return CharSet(" \t\v\r"); }
    auto           WhiteSpace( void) const { return +WhiteChars(); }
    auto           OptWhiteSpace( void) const { return *WhiteChars(); }
    auto           NL( void) const { return !(Char('\r')) >> Char('\n'); }
    auto           OptBlankSpace( void) const { return *(WhiteChars() | NL()); }
    auto           BlankLine( void) const { return OptWhiteSpace() >> (NL() | EoS()); }
    auto           Comment( void) const { return Str( "<!--") >> *( Any() - Str( "-->")) >> Str( "-->"); } 

    auto           Reference( void) const { return Char( '&') >> ( Str( "quot") [ QuotListener()] |
                                                        Str( "apos") [  AposListener()] |
                                                        Str( "amp")  [  AmpListener()] |
                                                        Str( "lt")   [  LtListener()] |
                                                        Str( "gt")   [  GtListener()] |
                                                        Error( "unkown entity")
                                                        ) >> Char( ';'); }

    auto           Data( void) const { return Lexeme( (+ (Any() - (* Blank() >> Char( '<') | Char( '&')))) [ StringListener()] | Reference() ) ; }  

    auto           CData( void) const { return ( Str( "<![CDATA[") >> Lexeme( * (Any() -Str( "]]>")))[ StringListener()] >> Str( "]]>"))[ DataOver()]; }  

    auto           Ident( void) const { return Lexeme( (CharSet( "a-zA-Z_:") | CharRange(0x80, 0xFF)) >> * ( CharSet( "a-zA-Z0-9_.:-") | CharRange(  0x80, 0xFF))); }  
    
    auto           Attribute( void) const { return ( Ident() [ AttributeNameListener()] >> Char( '=') >> 
                            Lexeme( Char( '\"') >> * ((+ (~CharSet( "\"<&"))) [ StringListener()] | Reference()) >> Char( '\"') |
                                  Char( '\"') >> * ((+ (~CharSet( "\'<&"))) [ StringListener()] |
                                                    Reference()) >> Char( '\''))) [ AttributeListener()]; }
    auto           LeafElement( void) const { return ( Char( '<') >> OptBlankSpace() >> Ident()[ ElementNameListener()] >> * (  OptBlankSpace() >> Attribute()) >> OptBlankSpace() >> Str( "/>")) [ ElementLeafOver()]; }
    
    auto           ElementEnd( void) const { return ( Str( "</") >>  OptBlankSpace() >> Ident()[ ElementNameListener()] >>  OptBlankSpace() >> Char( '>'))[ ElementBranchOver()]; }
    auto           ElementBegin( void) const { return ( Char( '<') >>  OptBlankSpace() >> Ident()[ ElementNameListener()] >> * (  OptBlankSpace() >> Attribute() ) >> OptBlankSpace() >> Char( '>'))[ ElementBranchBegin()]; }

	auto			Elem( void) const { return  ( LeafElement() | Comment() |  CData()) | ( ElementBegin() >> *(  OptBlankSpace() >> Ref( SELF) ) >> OptBlankSpace() >> ElementEnd()); }

 
template < typename Forge>
    bool    DoParse( Forge *ctxt) const
    {    
        auto    elem = Elem();
        if (  !elem.DoMatch( ctxt))
            return false;  
        return true;
    }

template < typename Cnstr>
	auto        FetchElemId( Cnstr *cnstr)
	{  
		auto		elem = new XMLSynElem();  
		elem->m_LockFlg = 0;
		Cv_CrateId	crateId = cnstr->Store( elem);
		auto		node = Elem();
		elem->m_Item = cnstr->FetchElemId( &node);
		return crateId;
	} 
};

//_____________________________________________________________________________________________________________________________

struct XMLDoc  : public Shard< XMLDoc>
{  
   
    XMLDoc( void) 
    {}

    auto           DocumentOver( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt->MatchStr() << "\n";
                                                        return true;  };  } 

    auto           PI( void) const { return Str( "<?") >> *( Any() - Str( "?>")) >> Str( "?>"); } 
 
    auto           Document( void) const { return +( XMLElement() | PI() )[ DocumentOver()]; } 

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
		auto	elem = new XDocSynElem();  
		auto	node = Document();
		elem->m_Item = cnstr->FetchElemId( &node);
		return cnstr->Store( elem);
	} 
};

//_____________________________________________________________________________________________________________________________ 

typedef Cv_Crate< XDocSynElem, XMLSynElem,  SynParserCrate>   XmlParserCrate; 

//_____________________________________________________________________________________________________________________________
};
