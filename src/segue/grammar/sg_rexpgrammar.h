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
struct      RExpQuanta;
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
        return [](auto forge) {
            forge->Pred< RExpUnit>()->m_ChSet.Set( forge->MatchStr()[0], true);
            return true;  }; }

    auto		CtrlCharListener(void) const {
        return [](auto forge) {
            forge->Pred< RExpUnit>()->m_ChSet.Set( forge->MatchStr()[0] -'a', true);
            return true;  }; }

    auto		EscCharListener(void) const {
        return [](auto forge) {
            forge->Pred< RExpUnit>()->m_ChSet.Set( forge->MatchStr()[0], true);
            return true;  }; }

    auto		OctalListener(void) const {
        return [](auto forge) {
            forge->Pred< RExpUnit>()->m_ChSet.Set( forge->num, true);
            return true;  }; }

    auto		HexListener(void) const {
        return [](auto forge) {
            forge->Pred< RExpUnit>()->m_ChSet.Set( forge->num, true);
            return true;  }; }

    auto		BackrefListener(void) const {
        return [](auto forge) {
            std::cout << forge->MatchStr() << "\n";
            return true;  }; }

    auto		WhiteSpaceListener(void) const {
        return [](auto forge) {
            forge->Pred< RExpUnit>()->m_ChSet = Sg_ChSet::Space();
            return true;  }; }

    auto		NonWhiteSpaceListener(void) const {
        return [](auto forge) {
            forge->Pred< RExpUnit>()->m_ChSet = Sg_ChSet::NonSpace();
            return true;  }; }

    auto		DigitListener(void) const {
        return [](auto forge) {
            forge->Pred< RExpUnit>()->m_ChSet = Sg_ChSet::Digit();
            return true;  }; }

    auto		NonDigitListener(void) const {
        return [](auto forge) {
            forge->Pred< RExpUnit>()->m_ChSet = Sg_ChSet::NonDigit();
            return true;  }; }

    auto		WordListener(void) const {
        return [](auto forge) {
            forge->Pred< RExpUnit>()->m_ChSet = Sg_ChSet::Word();
            return true;  }; } 

    auto		NonWordListener(void) const {
        return [](auto forge) {
            forge->Pred< RExpUnit>()->m_ChSet = Sg_ChSet::NonWord();
            return true;  }; }

    auto		WordBdyListener(void) const {
        return [](auto forge) {
            return true;  }; }

    auto		NonWordBdyListener(void) const {
        return [](auto forge) {
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



    auto        Unit(  void) const { return m_AlphaNum[ CharListener()] | 
        ( Char('\\') >> ( KeyChar() | CharSet("abfnrtv")[ CtrlCharListener()] | m_EscapedCharset[ EscCharListener()]  | 
            m_Octal[ OctalListener()] | (IStr( "x") >> m_Hex[ HexListener()]) | m_BackRef[ BackrefListener()] )); } 

    template < typename Forge>
    bool    DoParse( Forge *forge) const
    {
        forge->Push();
        auto	unit = Unit();
        if ( !unit.DoMatch( forge))
            return false; 
        return true;
    }
}; 

//_____________________________________________________________________________________________________________________________

struct RExpSeq : public Shard< RExpSeq>, public RExpPrimitive
{ 
    struct Whorl
    {  
        std::vector< RExpCrate::Id>		m_RExps;  

        RExpCrate::Id           FetchId( RExpRepos *repos) 
        {
            auto    synElem = new SeqSynElem();
            synElem->m_SeqList = m_RExps; 
            return repos->Store( synElem);        
        }
    };

    auto        QuantaListener(void) const {
        return [this]( auto forge) {                      // build seq
            auto    docWhorl = forge->Bottom< RExpDoc>();
            forge->Pred< RExpSeq>()->m_RExps.push_back( forge->FetchId( docWhorl->m_Repos)); 
            return true;  }; } 
     
    auto	Seq( void) const;

template < typename Forge>
    bool    DoParse( Forge *forge) const
    {
        forge->Push();
        auto	unit = Seq();
        if ( !unit.DoMatch( forge))
            return false; 
        return true;
    }
};

//_____________________________________________________________________________________________________________________________

struct RExpAtom : public Shard< RExpAtom>, public RExpPrimitive
{
    RExpSeq             m_Seq;
    RExpUnit            m_RExpUnit; 

    struct Whorl
    { 
        uint32_t                m_ReposSz;
        RExpCrate::Id           m_Id; 
        Sg_ChSet                m_ChSet;
 
        Whorl(void)
            : m_ReposSz( 0) 
        {}

    template < typename Parser>
        void Initialize( Parser *parser)
        {
            auto    docWhorl = parser->Bottom< RExpDoc>(); 
            m_ReposSz  = docWhorl->m_Repos->Size(); 
        }

    template < typename Parser>
        void Scavenge( Parser *parser)
        {
             m_Repos->Shrivel( m_ReposSz); 
        }

        RExpCrate::Id           FetchId( RExpRepos *repos) 
        {
            if ( m_Id.IsValid()) 
                return m_Id;
            auto    synElem = new CSetSynElem();
            synElem->m_Filt = m_ChSet; 
            return repos->Store( synElem);
        }
    };
 
    auto        UnitListener(void) const {
        return [this]( auto forge) {
            forge->Pred< RExpQuanta>()->m_ChSet = forge->m_ChSet;

            std::cout << forge->MatchStr() << "\n";
            return true;  }; } 

    auto        SeqListener(void) const {
        return [this]( auto forge) { 
            std::cout << forge->MatchStr() << "\n"; 
            auto            docWhorl = forge->Bottom< RExpDoc>();
            forge->Pred< RExpQuanta>()->m_Id = forge->FetchId( docWhorl->m_Repos);
            return true;  }; }

    auto        RexpListener(void) const {
        return [this]( auto forge) { 
            std::cout << forge->MatchStr() << "\n"; 
            return true;  }; }

    auto		Atom( void) const { 
        return ( Char('(') >> m_Seq[ SeqListener()] >> Char(')'))[ RexpListener()] | m_RExpUnit[ UnitListener()]; } 

template < typename Forge>
    bool    DoParse( Forge *forge) const
    {
        forge->Push();
        auto	unit = Atom();
        if ( !unit.DoMatch( forge))
            return false; 
        return true;
    }
};

//_____________________________________________________________________________________________________________________________

struct RExpQuanta : public Shard< RExpQuanta>, public RExpPrimitive
{  
    RExpAtom                    m_Atom;
    ParseInt< uint32_t, 10>	    m_SpinMin;
    ParseInt< uint32_t, 10>     m_SpinMax;  
    struct Whorl
    {    
        Sg_ChSet                m_ChSet;
        uint32_t		        m_Min;
        uint32_t		        m_Max;
        bool                    m_Infinite;
        bool                    m_Stingy;
        RExpCrate::Id           m_Id; 

        Whorl(void)
            :  m_Min( CV_UINT32_MAX), m_Max( CV_UINT32_MAX), m_Infinite( false), m_Stingy( false) 
        {} 

    template < typename Parser>
        void Scavenge( Parser *parser)
        {
            auto    docWhorl = parser->Bottom< RExpDoc>(); 
        }

        bool            IsBasic( void) const 
        { 
            return ( m_Min == CV_UINT32_MAX) && ( m_Max == CV_UINT32_MAX) && !m_Infinite && !m_Stingy; 
        }

        RExpCrate::Id           FetchId( RExpRepos *repos) 
        {
            if ( m_Id.IsValid()) 
            {
                auto    synElem = new CSetSynElem();
                synElem->m_Filt = m_ChSet; 
                m_Id = repos->Store( synElem);
            }
            CV_ERROR_ASSERT( IsBasic())
            return m_Id;
        }
    };
  
    auto		MinSpinListener(void) const {
        return []( auto forge) {
            Whorl       *whorl = forge->Pred< RExpQuanta>();
            whorl->m_Min = forge->num;
            whorl->m_Max = forge->num; 
            return true;  }; }

    auto		MaxSpinListener(void) const {
        return []( auto forge) {
            Whorl       *whorl = forge->Pred< RExpQuanta>();
            whorl->m_Max = forge->num;
            whorl->m_Infinite = false;
            return true;  }; }

    auto		CommaListener(void) const {
        return [](auto forge) {
            Whorl       *whorl = forge->Pred< RExpQuanta>();
            whorl->m_Max = 0;
            whorl->m_Infinite = true;
            return true;  }; }

    auto		ZeroToMaxListener(void) const {
        return []( auto forge) {
            Whorl       *whorl = forge->Pred< RExpQuanta>();
            whorl->m_Min = 0;
            whorl->m_Max = forge->num; 
            return true;  }; }

    auto		QuestionListener(void) const {
        return []( auto forge) {
            Whorl       *whorl = forge->Pred< RExpQuanta>();
            whorl->m_Min = 0;
            whorl->m_Max = 1;
            whorl->m_Infinite = false;
            return true;  }; }

    auto		StarListener(void) const {
        return []( auto forge) {
            Whorl       *whorl = forge->Pred< RExpQuanta>();
            whorl->m_Min = 0;
            whorl->m_Max = 0;
            whorl->m_Infinite = true;
            return true;  }; }

    auto		PlusListener(void) const {
        return []( auto forge) {
            Whorl       *whorl = forge->Pred< RExpQuanta>();
            whorl->m_Min = 1;
            whorl->m_Max = 0;
            whorl->m_Infinite = true;
            return true;  }; }

    auto	    StingyListener(void) const {
        return [](auto forge) {
            Whorl       *whorl = forge->Pred< RExpQuanta>();
            whorl->m_Stingy = true;
            return true;  }; } 

    auto	    AtomListener(void) const {
        return [](auto forge) {
            Whorl       *whorl = forge->Pred< RExpQuanta>(); 
            auto        docWhorl = forge->Bottom< RExpDoc>();
            whorl->m_Id = forge->FetchId( docWhorl->m_Repos);
            return true;  }; } 

    auto		Quanta( void) const { 
        return m_Atom[ AtomListener()] >> !(( Char('{') >> m_SpinMin[ MinSpinListener()] >> !(Char(',')[ CommaListener()] >> !(m_SpinMax[ MaxSpinListener()])) >> Char('}')) |
            ( Char('{') >> Char(',') >> (m_SpinMax[ZeroToMaxListener()]) >> Char('}')) |
            ( Char('?')[ QuestionListener()] | Char('*')[ StarListener()] | Char('+')[ PlusListener()] >> !Char('?')[ StingyListener()])) ; } 

template < typename Forge>
    bool    DoParse( Forge *forge) const
    {
        forge->Push();
        auto	unit = Quanta();
        if ( !unit.DoMatch( forge))
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
        
    

        RExpCrate::Id           FetchId( RExpRepos *repos) 
        {
            auto    synElem = new SeqSynElem();
            synElem->m_SeqList = m_RExps; 
            return repos->Store( synElem);  
        }
	};
 
	auto           RExpressionListener(void) const {
		return []( auto forge) {
			std::cout << forge->MatchStr() << "\n"; 
			return true;  }; }

	auto          IndexListener(void) const {
		return [this]( auto forge) {   
 		    forge->Pred< RExpEntry>()->m_Index = forge->num;
			return true;  }; }

	auto          AtomListener(void) const {
		return [this]( auto forge) { 
            std::cout << forge->MatchStr() << "\n";
            auto        docWhorl = forge->Bottom< RExpDoc>();
 	        forge->Pred< RExpEntry>()->m_RExps.push_back( forge->FetchId( docWhorl->m_Repos));
			return true;  }; }

	auto           RExpression(void) const { return (+(RExpQuanta()[ AtomListener()] - Char('/')))[ RExpressionListener()]; }
	auto           RExpEnd( void) const { return Char( '/')  ; }
	auto           RExpBegin( void) const { return ( Char( '/') >>  OptBlankSpace()); }
	auto		   RExpLine(void) const { return  ( Comment()  | ( OptBlankSpace()  >> ParseInt<uint64_t>()[IndexListener()] >> Char(',') >> RExpBegin() >> RExpression() >> RExpEnd() >> BlankLine())); }

template < typename Forge>
	bool    DoParse( Forge *forge) const
	{     
		forge->Push();
		auto	rexpLine = RExpLine( ); 
		if (!rexpLine.DoMatch(forge))
			return false;
		std::cout << forge->m_Index << "\n";
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
    RExpRepos       *m_Repos;
    struct XAct
    {
        RExpCrate::Id   m_Id;

        XAct( void) 
        {}
    };
 
	struct Whorl
	{
        RExpRepos                       *m_Repos; 
        std::vector< RExpCrate::Id>     m_RExps;
        
    template <typename ParentForge>
        bool    PrimeUp( ParentForge *xact)
        {
            m_Repos = xact->m_Repos;
            return true;
        }

        RExpCrate::Id           FetchId( RExpRepos *repos) 
        {
            auto    synElem = new AltSynElem();
            synElem->m_AltList = m_RExps; 
            return repos->Store( synElem);        
        }
	};

	RExpDoc( RExpRepos *repos)
        : m_Repos( repos)
    {}  

    auto          RExpListener(void) const {
        return [this]( auto forge) {  
            auto            docWhorl = forge->Pred< RExpDoc>();
            docWhorl->m_RExps.push_back( forge->FetchId( docWhorl->m_Repos));
            return true;  }; }

    
	auto           DocumentOver( void) const { return [ this]( auto forge) { 
        auto            docWhorl = forge->Bottom< RExpDoc>(); 
        RExpCrate::Id   id = docWhorl->FetchId( docWhorl->m_Repos);
		return true;  };  } 
	 
	auto           Document( void) const { return (+RExpEntry()[ RExpListener()] )[ DocumentOver()]; } 

template < typename Forge>
	bool    DoParse( Forge *forge) const
	{   
        forge->m_Repos = m_Repos;
		forge->Push();
		auto    doc = Document();
		if (  !doc.DoMatch( forge))
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


inline auto	RExpSeq::Seq( void) const { 
    RExpQuanta  quanta;
    return  (+( quanta[ QuantaListener()])); }
};
