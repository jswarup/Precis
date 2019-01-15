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

struct      RExpDoc;

struct      RExpDocSynElem;
struct      RExpSynElem;

typedef Cv_Crate< RExpDocSynElem, RExpSynElem,  SynParserCrate>     RExpCrate; 
typedef Cv_CrateRepos< RExpCrate>                                   RExpRepos;

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
    typedef ParseInt< uint8_t, 8, 2, 3>     Oct;
    typedef ParseInt< uint16_t, 10, 1, 3>   Dec;
    typedef ParseInt< uint8_t, 16, 0, 2>    Hex;

    CharSet     m_AlphaNum;
    CharSet     m_EscapedCharset;
    Oct         m_Octal;
    Hex         m_Hex;
    Dec         m_BackRef;

    static constexpr const char   *EscapedCharset = ".^$*+?()[{\\|";    // "[]+*?{}().\\/"

    struct Whorl
    {
        Sg_ChSet		m_ChSet;
    }; 
    
    RExpUnit( void)
        :  m_AlphaNum( AlphaNum()), m_EscapedCharset( EscapedCharset ), m_Octal()
    {}
    
    auto		CharListener(void) const {
        return [](auto ctxt) {
            ctxt->Pred< RExpUnit>()->m_ChSet.Set( ctxt->MatchStr()[0], true);
            return true;  }; }

    auto		CtrlCharListener(void) const {
        return [](auto ctxt) {
            ctxt->Pred< RExpUnit>()->m_ChSet.Set( ctxt->MatchStr()[0] -'a', true);
            return true;  }; }

    auto		EscCharListener(void) const {
        return [](auto ctxt) {
            ctxt->Pred< RExpUnit>()->m_ChSet.Set( ctxt->MatchStr()[0], true);
            return true;  }; }

    auto		OctalListener(void) const {
        return [](auto ctxt) {
            ctxt->Pred< RExpUnit>()->m_ChSet.Set( ctxt->num, true);
            return true;  }; }

    auto		HexListener(void) const {
        return [](auto ctxt) {
            ctxt->Pred< RExpUnit>()->m_ChSet.Set( ctxt->num, true);
            return true;  }; }

    auto		BackrefListener(void) const {
        return [](auto ctxt) {
            std::cout << ctxt->MatchStr() << "\n";
            return true;  }; }

    auto		WhiteSpaceListener(void) const {
        return [](auto ctxt) {
            ctxt->Pred< RExpUnit>()->m_ChSet = Sg_ChSet::Space();
            return true;  }; }

    auto		NonWhiteSpaceListener(void) const {
        return [](auto ctxt) {
            ctxt->Pred< RExpUnit>()->m_ChSet = Sg_ChSet::NonSpace();
            return true;  }; }

    auto		DigitListener(void) const {
        return [](auto ctxt) {
            ctxt->Pred< RExpUnit>()->m_ChSet = Sg_ChSet::Digit();
            return true;  }; }

    auto		NonDigitListener(void) const {
        return [](auto ctxt) {
            ctxt->Pred< RExpUnit>()->m_ChSet = Sg_ChSet::NonDigit();
            return true;  }; }

    auto		WordListener(void) const {
        return [](auto ctxt) {
            ctxt->Pred< RExpUnit>()->m_ChSet = Sg_ChSet::Word();
            return true;  }; } 

    auto		NonWordListener(void) const {
        return [](auto ctxt) {
            ctxt->Pred< RExpUnit>()->m_ChSet = Sg_ChSet::NonWord();
            return true;  }; }

    auto		WordBdyListener(void) const {
        return [](auto ctxt) {
            return true;  }; }

    auto		NonWordBdyListener(void) const {
        return [](auto ctxt) {
            return true;  }; } 

    auto           KeyChar( void) const {
        return  Char('s')[ WhiteSpaceListener()] |
            Char('S')[ NonWhiteSpaceListener()] |
            Char('d')[ DigitListener()] |
            Char('D')[ NonDigitListener()] |
            Char('w')[ WordListener()] |
            Char('W')[ NonWordListener()] |
            Char('b')[ WordBdyListener()] |
            Char('B')[ NonWordBdyListener()];  }

    auto          UnitListener(void) const {
        return [this]( auto ctxt) {
            ctxt->Pred< RExpEntry>()->m_ChSets.push_back( ctxt->m_ChSet);
            return true;  }; }

    auto        Unit(  void) const { return m_AlphaNum[ CharListener()] | 
        ( Char('\\') >> ( KeyChar() | CharSet("abfnrtv")[ CtrlCharListener()] | m_EscapedCharset[ EscCharListener()]  | 
            m_Octal[ OctalListener()] | (IStr( "x") >> m_Hex[ HexListener()]) | m_BackRef[ BackrefListener()] )); } 

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

struct RExpSingleton : public Shard< RExpSingleton>, public RExpPrimitive
{
    RExpUnit                    m_RExpUnit;
    ParseInt< uint32_t, 10>	    m_SpinMin;
    ParseInt< uint32_t, 10>     m_SpinMax;
    
    struct Whorl
    {  
        RExpRepos               *m_Repos;  
        Sg_ChSet                m_ChSet;
        uint32_t		        m_Min;
        uint32_t		        m_Max;
        bool                    m_Infinite;
        bool                    m_Stingy;
        RExpCrate::Id           m_Id; 

        Whorl(void)
            : m_Repos( NULL), m_Min( CV_UINT32_MAX), m_Max( CV_UINT32_MAX), m_Infinite( false), m_Stingy( false) 
        {
        } 

template < typename Parser>
        void Initialize( Parser *parser)
        {
            auto    docWhorl = parser->Bottom< RExpDoc>();
            m_Repos  = &docWhorl->m_Repos; 
        }

template < typename Parser>
        void Scavenge( Parser *parser)
        {
            auto    docWhorl = parser->Bottom< RExpDoc>();
            docWhorl->m_Repos.Shrivel( m_ReposSz);
            return;
        }

        bool            IsBasic( void) const 
            { return ( m_Min == CV_UINT32_MAX) && ( m_Max == CV_UINT32_MAX) && !m_Infinite && !m_Stingy; }

        RExpCrate::Id           FetchId( void) 
        {
            if ( !m_Id.IsValid()) 
            {
                auto    synElem = new CSetSynElem();
                synElem->m_Filt = m_ChSet; 
                m_Id = m_Repos->Store( synElem);
            }
            CV_ERROR_ASSERT( IsBasic())
            return m_Id;
        }
    };

    auto        UnitListener(void) const {
        return [this]( auto ctxt) {
            ctxt->Pred< RExpSingleton>()->m_ChSet = ctxt->m_ChSet;
            return true;  }; }

	auto        Singleton( const RExpSingleton *re) const { return  ( Char('(') >> (*re) >> Char(')')) | m_RExpUnit[ UnitListener()]; }

	auto		MinSpinListener(void) const {
		return []( auto ctxt) {
            Whorl       *whorl = ctxt->Pred< RExpSingleton>();
            whorl->m_Min = ctxt->num;
            whorl->m_Max = ctxt->num;
			return true;  }; }

	auto		MaxSpinListener(void) const {
		return []( auto ctxt) {
            Whorl       *whorl = ctxt->Pred< RExpSingleton>();
            whorl->m_Max = ctxt->num;
            whorl->m_Infinite = false;
			return true;  }; }

	auto		CommaListener(void) const {
		return [](auto ctxt) {
            Whorl       *whorl = ctxt->Pred< RExpSingleton>();
            whorl->m_Max = 0;
            whorl->m_Infinite = true;
			return true;  }; }

	auto		ZeroToMaxListener(void) const {
		return []( auto ctxt) {
            Whorl       *whorl = ctxt->Pred< RExpSingleton>();
            whorl->m_Min = 0;
            whorl->m_Max = ctxt->num; 
			return true;  }; }

	auto		QuestionListener(void) const {
		return []( auto ctxt) {
            Whorl       *whorl = ctxt->Pred< RExpSingleton>();
            whorl->m_Min = 0;
            whorl->m_Max = 1;
            whorl->m_Infinite = false;
			return true;  }; }

	auto		StarListener(void) const {
		return []( auto ctxt) {
            Whorl       *whorl = ctxt->Pred< RExpSingleton>();
            whorl->m_Min = 0;
            whorl->m_Max = 0;
            whorl->m_Infinite = true;
			return true;  }; }

	auto		PlusListener(void) const {
		return []( auto ctxt) {
            Whorl       *whorl = ctxt->Pred< RExpSingleton>();
            whorl->m_Min = 1;
            whorl->m_Max = 0;
            whorl->m_Infinite = true;
			return true;  }; }

	auto		StingyListener(void) const {
		return [](auto ctxt) {
            Whorl       *whorl = ctxt->Pred< RExpSingleton>();
            whorl->m_Stingy = true;
			return true;  }; }

	auto		Quanta( const RExpSingleton *re) const {

		return Singleton( re) >> !(( Char('{') >> m_SpinMin[ MinSpinListener()] >> !(Char(',')[ CommaListener()] >> !(m_SpinMax[ MaxSpinListener()])) >> Char('}')) |
			              ( Char('{') >> Char(',') >> (m_SpinMax[ZeroToMaxListener()]) >> Char('}')) |
		                  ( Char('?')[ QuestionListener()] | Char('*')[ StarListener()] | Char('+')[ PlusListener()] >> !Char('?')[ StingyListener()])) ; }

template < typename Forge>
	bool    DoParse( Forge *ctxt) const
	{
		ctxt->Push();
		auto	unit = Quanta( this);
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
		uint64_t					    m_Index;
		std::vector< RExpCrate::Id>		m_RExps;

		Whorl( void)
			: m_Index( 0)
		{}

		~Whorl(void)
		{
            
        }
	};

	RExpEntry( void) 
	{} 

	auto           RExpressionListener(void) const {
		return []( auto ctxt) {
			std::cout << ctxt->MatchStr() << "\n";
			return true;  }; }

	auto          IndexListener(void) const {
		return [this]( auto ctxt) {   
			ctxt->Pred< RExpEntry>()->m_Index = ctxt->num;
			return true;  }; }

	auto          UnitListener(void) const {
		return [this]( auto ctxt) {
	        ctxt->Pred< RExpEntry>()->m_RExps.push_back( ctxt->FetchId());
			return true;  }; }

	auto           RExpression(void) const { return (+(RExpSingleton()[ UnitListener()] - Char('/')))[ RExpressionListener()]; }
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
        Cv_CrateRepos< RExpCrate>       m_Repos;
		AltSynElem	                    m_AllRules;
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
};
