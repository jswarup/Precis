// sg_xmlgrammar.h ___________________________________________________________________________________________________________________
#pragma once


#include    "segue/timbre/sg_timbreparser.h"
#include 	"segue/xml/sg_xmlnode.h" 

namespace Sg_Xml
{

using namespace Sg_Timbre; 

//_____________________________________________________________________________________________________________________________

struct XMLElement : public Node< XMLElement>
{

    XMLElement( void)
    {
    }

    auto           ElementNameListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt.MatchStr() << "\n"; 
                                                        return true;  };  }

    auto           ElementLeafOver( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt.MatchStr() << "\n"; 
                                                        return true;  };  }

    auto           ElementBranchOver( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt.MatchStr() << "\n"; 
                                                        return true;  };  }

    auto           ElementBranchBegin( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt.MatchStr() << "\n"; 
                                                        return true;  };  }

    auto           AttributeNameListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt.MatchStr() << "\n"; 
                                                        return true;  };  }
    
    auto           StringListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt.MatchStr() << "\n"; 
                                                        return true;  };  }

    auto           QuotListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt.MatchStr() << "\n"; 
                                                        return true;  };  }

    
    auto           AttributeListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt.MatchStr() << "\n"; 
                                                        return true;  };  }

    auto           AposListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt.MatchStr() << "\n"; 
                                                        return true;  };  }

    auto           AmpListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt.MatchStr() << "\n"; 
                                                        return true;  };  }

    auto           LtListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt.MatchStr() << "\n"; 
                                                        return true;  };  }

    auto           GtListener( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt.MatchStr() << "\n"; 
                                                        return true;  };  }


    auto           DataOver( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt.MatchStr() << "\n"; 
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

    auto           Ident( void) const { return Lexeme( (CharSet( "a-zA-Z_:") | Range<(char) 0x80, (char) 0xFF>()) >> * ( CharSet( "a-zA-Z0-9_.:-") | Range<(char) 0x80, (char) 0xFF>())); }  
    
    auto           Attribute( void) const { return ( Ident() [ AttributeNameListener()] >> Char( '=') >> 
                            Lexeme( Char( '\"') >> * ((+ (~CharSet( "\"<&"))) [ StringListener()] | Reference()) >> Char( '\"') |
                                  Char( '\"') >> * ((+ (~CharSet( "\'<&"))) [ StringListener()] |
                                                    Reference()) >> Char( '\''))) [ AttributeListener()]; }
    auto           LeafElement( void) const { return ( Char( '<') >> OptBlankSpace() >> Ident()[ ElementNameListener()] >> * (  OptBlankSpace() >> Attribute()) >> OptBlankSpace() >> Str( "/>")) [ ElementLeafOver()]; }
    
    auto           ElementEnd( void) const { return ( Str( "</") >>  OptBlankSpace() >> Ident()[ ElementNameListener()] >>  OptBlankSpace() >> Char( '>'))[ ElementBranchOver()]; }
    auto           ElementBegin( void) const { return ( Char( '<') >>  OptBlankSpace() >> Ident()[ ElementNameListener()] >> * (  OptBlankSpace() >> Attribute() ) >> OptBlankSpace() >> Char( '>'))[ ElementBranchBegin()]; }
 
template < typename Forge>
    bool    DoParse( Forge *ctxt) const
    {    
        auto    elem = ( LeafElement() | Comment() |  CData()) | ( ElementBegin() >> *(  OptBlankSpace() >> XMLElement() ) >> OptBlankSpace() >> ElementEnd());
        if (  !elem.DoMatch( ctxt))
            return false;  
        return true;
    }
    
    struct     SynElem : public Sg_Timbre::SynElem
    {
     
    };
};

//_____________________________________________________________________________________________________________________________

struct XMLDoc  : public Node< XMLDoc>
{ 
    Cv_Repos< SynParserCrate::Entry>              m_SynRepos;
    Cv_CrateRepos < SynParserCrate>               m_SynCrate;
    Cv_CrateRepos < SynParserCrate>::Constructor  m_SynCnstr; 

    XMLDoc( void)
        : m_SynRepos(),  m_SynCrate( &m_SynRepos), m_SynCnstr( &m_SynCrate)
    {
    }

    
    auto           DocumentOver( void) const { return []( auto ctxt) {  
                                                            std::cout << ctxt.MatchStr() << "\n";
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
    
    struct     SynElem : public Sg_Timbre::SynElem
    {
     
    };
};

//_____________________________________________________________________________________________________________________________
};
