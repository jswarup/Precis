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

    auto        AlphaEx( void) const { return CharSet( "a-zA-Z_"); }
    auto        AlphaNum( void) const { return CharSet( "a-zA-Z0-9"); } 

	auto        Blank(void) const { return CharSet(" \t\v\r\n"); }
	auto        WhiteChars(void) const { return CharSet(" \t\v\r"); }
	auto        WhiteSpace(void) const { return +WhiteChars(); }
	auto        OptWhiteSpace(void) const { return *WhiteChars(); }
	auto        NL(void) const { return !(Char('\r')) >> Char('\n'); }
	auto        OptBlankSpace(void) const { return *(WhiteChars() | NL()); }
	auto        BlankLine(void) const { return OptWhiteSpace() >> (NL() | EoS()); }
	auto        Comment(void) const { return Str("<!--") >> *(Any() - Str("-->")) >> Str("-->"); } 
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

struct RExpNamedCharClass : public Shard< RExpNamedCharClass>, public RExpPrimitive
{
    struct Whorl
    {
        Sg_ChSet    m_ChSet;
        bool        m_NegateFlg;
        Whorl( void)
            : m_NegateFlg( false)
        {}        

        auto        FetchCCL( void) const { return m_NegateFlg ? m_ChSet.Negative() : m_ChSet; }
        void        SetName( const Cv_CStr &name)
        {
                if ( name == "alnum")
                    m_ChSet = Sg_ChSet::AlphaNum();
                else if ( name == "alpha")
                    m_ChSet = Sg_ChSet::Alpha();
                else if ( name == "ascii")
                    m_ChSet = Sg_ChSet::Ascii();
                else if ( name == "blank")
                    m_ChSet = Sg_ChSet::Blank();
                else if ( name == "cntrl")
                    m_ChSet = Sg_ChSet::Cntrl();
                else if ( name == "digit")
                    m_ChSet = Sg_ChSet::Digit();
                else if ( name == "graph")
                    m_ChSet = Sg_ChSet::Graph();
                else if ( name == "lower")
                    m_ChSet = Sg_ChSet::Lower();
                else if ( name == "print")
                    m_ChSet = Sg_ChSet::Print();
                else if ( name == "punct")
                    m_ChSet = Sg_ChSet::Punct();
                else if ( name == "space")
                    m_ChSet = Sg_ChSet::Space();
                else if ( name == "upper")
                    m_ChSet = Sg_ChSet::Upper();
                else if ( name == "word")
                    m_ChSet = Sg_ChSet::Word();
                else if ( name == "xdigit")
                    m_ChSet = Sg_ChSet::XDigit(); 
        }
    };
    
    auto		NameListener( void) const {  
        return []( auto forge) {  
            forge->Pred< RExpNamedCharClass>()->SetName( forge->MatchStr()); 
            return true;  }; } 

    auto		NegateListener( void) const {
        return []( auto forge) {
            forge->Pred< RExpNamedCharClass>()->m_NegateFlg = true;
            return true;  }; }

    auto        NamedCCL(  void) const { return  Str( "[:" ) >> !( Char( '^')[ NegateListener()]) >> (+AlphaEx())[ NameListener()] >> Str( ":]" ); }

template < typename Forge>
    bool    DoParse( Forge *forge) const
    {
        forge->Push();
        auto	namedCCL = NamedCCL();
        if ( !namedCCL.DoMatch( forge))
            return false; 
        return true;
    } 
    void    Dump( std::ostream &ostr) const
    {
        return;
    } 
};

//_____________________________________________________________________________________________________________________________

struct RExpAnyCCLChar : public Shard< RExpAnyCCLChar>, public RExpPrimitive
{ 
    typedef ParseInt< uint8_t, 8, 3, 3>     Oct;
    typedef ParseInt< uint16_t, 10, 1, 3>   Dec;
    typedef ParseInt< uint8_t, 16, 2, 2>    Hex;

    static constexpr const char   *s_CCLEscapedCharset = "^-]\\";    // "[]+*?{}().\\/";
    struct Whorl
    {
        uint8_t     m_Char;

        Whorl( void)
            : m_Char( 0)
        {}
    }; 
    
    friend std::ostream &operator<<( std::ostream &ostr, const RExpAnyCCLChar &shard)
    {
        return ostr;
    }   
    
    auto		CharListener( void) const {
        return []( auto forge) {
            forge->Pred< RExpAnyCCLChar>()->m_Char = forge->MatchStr()[0];
            return true;  }; }
    auto		AlphaNumListener( void) const {
        return []( auto forge) {
            forge->Pred< RExpAnyCCLChar>()->m_Char = forge->MatchStr()[0];
            return true;  }; }

    auto		CtrlCharListener( void) const {
        return []( auto forge) {
            uint8_t     *chPtr = &forge->Pred< RExpAnyCCLChar>()->m_Char; 
            switch ( forge->MatchStr()[0])
            {
                case 'a': *chPtr = '\a'; break;
                case 'b': *chPtr = '\b'; break;
                case 'f': *chPtr = '\f'; break;
                case 'n': *chPtr = '\n'; break;
                case 'r': *chPtr = '\r'; break;
                case 't': *chPtr = '\t'; break;
                case 'v': *chPtr = '\v'; break;
            }
            return true;  }; }

    auto		CharValueListener( void) const {
        return []( auto forge) {
            forge->Pred< RExpAnyCCLChar>()->m_Char = uint8_t( forge->num);
            return true;  }; }

    auto        AnyCCLChar(  void) const { 
        return AlphaNum()[ AlphaNumListener()] | ( Char( '\\') >> ( CharSet( "abfnrtv")[ CtrlCharListener()] |  CharSet( s_CCLEscapedCharset)[ CharListener()] |  
                        Oct()[ CharValueListener()] |  ( IStr( "x") >> Hex()[ CharValueListener()]) | Any()[ CharListener()])) | Any()[ CharListener()]; }

template < typename Forge>
    bool    DoParse( Forge *forge) const
    {
        forge->Push();
        auto	anyCharExp = AnyCCLChar();
        if ( !anyCharExp.DoMatch( forge))
            return false; 
        return true;
    }  
    void    Dump( std::ostream &ostr) const
    {
        return;
    } 
}; 


//_____________________________________________________________________________________________________________________________

struct RExpCCLCharRange : public Shard< RExpCCLCharRange>, public RExpPrimitive
{  
    RExpAnyCCLChar      m_AnyChar;

    struct Whorl
    {
        uint8_t     m_Beg;
        uint8_t     m_End;

        Whorl( void)
            : m_Beg( 0), m_End( 0)
        {}
        
        auto        FetchCCL( void) const 
        { 
            Sg_ChSet    chSet;
            for ( uint32_t i = m_Beg; i <= m_End; ++i)
                chSet.Set( i, true);
            return chSet; 
        }
    }; 
    
    auto		BegListener( void) const {
        return []( auto forge) {
            forge->Pred< RExpCCLCharRange>()->m_Beg = forge->m_Char;
            return true;  }; }

    auto		EndListener( void) const {
        return []( auto forge) {
            forge->Pred< RExpCCLCharRange>()->m_End = forge->m_Char;
            return true;  }; }

    auto        CCLCharRange(  void) const { 
        return ( m_AnyChar[ BegListener()] -Char(']')) >>  !( Char( '-') >> ( m_AnyChar[ EndListener()] -Char( ']'))); }


template < typename Forge>
    bool    DoParse( Forge *forge) const
    {
        forge->Push();
        auto	anyCharExp = CCLCharRange();
        if ( !anyCharExp.DoMatch( forge))
            return false; 
        return true;
    } 
    void    Dump( std::ostream &ostr) const
    {
        return;
    } 
};

//_____________________________________________________________________________________________________________________________

struct RExpCCL : public Shard< RExpCCL>, public RExpPrimitive
{
    RExpNamedCharClass      m_NamedCharClass;
    RExpCCLCharRange        m_CharRange;
         
    struct Whorl
    {
        Sg_ChSet    m_ChSet;
        bool        m_NegateFlg;
        
        Whorl( void)
            : m_NegateFlg( false)
        {}

        auto        FetchCCL( void) const { return m_NegateFlg ? m_ChSet.Negative() : m_ChSet; }
    }; 
    

    auto		NegateListener( void) const {
        return []( auto forge) {
            forge->Pred< RExpCCL>()->m_NegateFlg = true;
            return true;  }; }

    auto		SquareBracketEndListener( void) const {
        return []( auto forge) {
            forge->Pred< RExpCCL>()->m_ChSet.Set( ']', true);
            return true;  }; }

    auto		SpaceListener( void) const {
        return []( auto forge) {
            forge->Pred< RExpCCL>()->m_ChSet.UnionWith( Sg_ChSet::Space());
            return true;  }; }

    auto		NonSpaceListener( void) const {
        return []( auto forge) {
            forge->Pred< RExpCCL>()->m_ChSet.UnionWith( Sg_ChSet::NonSpace());
            return true;  }; }

    auto		DigitListener( void) const {
        return []( auto forge) {
            forge->Pred< RExpCCL>()->m_ChSet.UnionWith( Sg_ChSet::Digit());
            return true;  }; }
    
    auto		NonDigitListener( void) const {
        return []( auto forge) {
            forge->Pred< RExpCCL>()->m_ChSet.UnionWith( Sg_ChSet::NonDigit());
            return true;  }; }

    auto		WordListener( void) const {
        return []( auto forge) {
            forge->Pred< RExpCCL>()->m_ChSet.UnionWith( Sg_ChSet::Word());
            return true;  }; }


    auto		NonWordListener( void) const {
        return []( auto forge) {
            forge->Pred< RExpCCL>()->m_ChSet.UnionWith( Sg_ChSet::NonWord());
            return true;  }; }

    auto		NamedCharClassListener( void) const {
        return []( auto forge) {
            forge->Pred< RExpCCL>()->m_ChSet.UnionWith( forge->FetchCCL());
            return true;  }; }

    auto		CharRangeListener( void) const {
        return []( auto forge) {
            forge->Pred< RExpCCL>()->m_ChSet.UnionWith( forge->FetchCCL());
            return true;  }; }

    auto        CharClass(  void) const { return Char( '[') >> !( Char( '^')[ NegateListener()]) >> !( Char( ']')[ SquareBracketEndListener()]) >> 
                            *(( Char( '\\') >> ( Char( 's')[ SpaceListener()] | Char( 'S')[ NonSpaceListener()] | Char( 'd')[ DigitListener()] |
                                Char( 'D')[ NonDigitListener()] | Char( 'w')[ WordListener()] | Char( 'W')[ NonWordListener()])) | 
                                m_NamedCharClass[ NamedCharClassListener()] |  m_CharRange[ CharRangeListener()]) >>  Char( ']'); }

template < typename Forge>
    bool    DoParse( Forge *forge) const
    {
        forge->Push();
        auto	cclExp = CharClass();
        if ( !cclExp.DoMatch( forge))
            return false; 
        return true;
    }  
    void    Dump( std::ostream &ostr) const
    {
        ostr << "RExpCCL";
        return;
    } 
};

//_____________________________________________________________________________________________________________________________

struct RExpUnit : public Shard< RExpUnit>, public RExpPrimitive
{
    typedef ParseInt< uint8_t, 8, 2, 3>     Oct;
    typedef ParseInt< uint16_t, 10, 1, 3>   Dec;
    typedef ParseInt< uint8_t, 16, 2, 2>    Hex;

    CharSet     m_AlphaNum;
    CharSet     m_EscapedCharset;
    Oct         m_Octal;
    Hex         m_Hex;
    Dec         m_BackRef;
    RExpCCL     m_CCLExpr;

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

    auto		DotListener(void) const {
        return [](auto forge) {
            forge->Pred< RExpUnit>()->m_ChSet = Sg_ChSet::DotAll();
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

    auto		CCLExprListener(void) const {
        return [](auto forge) {
            forge->Pred< RExpUnit>()->m_ChSet = forge->FetchCCL();
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

    auto        Unit(  void) const { return m_AlphaNum[ CharListener()] | m_CCLExpr[ CCLExprListener()] | Char( '.')[ DotListener()] |
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
    void    Dump( std::ostream &ostr) const
    {
        ostr << "Unit";
        return;
    } 
}; 

//_____________________________________________________________________________________________________________________________

struct RExpAlt  : public Shard< RExpAlt>, public RExpPrimitive
{ 
    struct Whorl
    {  
        std::vector< RExpCrate::Id>		m_RExps;  

        RExpCrate::Id           FetchId( RExpRepos *repos) 
        {
            CV_ERROR_ASSERT( m_RExps.size())
            if ( m_RExps.size() == 1)
                return m_RExps[ 0];
            auto    synElem = new AltSynElem();
            synElem->m_AltList = m_RExps; 
            return repos->Store( synElem);        
        }
    };

    auto        QuantaListener(void) const {
        return [this]( auto forge) {                      // build seq
            auto    docWhorl = forge->Bottom< RExpDoc>();
            forge->Pred< RExpSeq>()->m_RExps.push_back( forge->FetchId( docWhorl->m_Repos)); 
            return true;  }; } 
      

    auto        AltCharListener(void) const {
        return [this]( auto forge) {                      // build seq
            auto    docWhorl = forge->Bottom< RExpDoc>(); 
            return true;  }; } 

    auto	Alt( void) const; 

template < typename Forge>
    bool    DoParse( Forge *forge) const
    {
        forge->Push();
        auto	unit = Alt();
        if ( !unit.DoMatch( forge))
            return false; 
        return true;
    }
    void    Dump( std::ostream &ostr) const
    {
        ostr << "Alt";
        return;
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
            CV_ERROR_ASSERT( m_RExps.size())
            if ( m_RExps.size() == 1)
                return m_RExps[ 0];
            auto    synElem = new SeqSynElem();
            synElem->m_SeqList = m_RExps; 
            return repos->Store( synElem);        
        }
    };

    auto        AltListener(void) const {
        return [this]( auto forge) {                      // build seq
            auto    docWhorl = forge->Bottom< RExpDoc>();
            forge->Pred< RExpSeq>()->m_RExps.push_back( forge->FetchId( docWhorl->m_Repos)); 
            return true;  }; } 


    auto	Seq( void) const {
        RExpAlt  alt;
        return  +( alt[ AltListener()]); }

template < typename Forge>
    bool    DoParse( Forge *forge) const
    {
        forge->Push();
        auto	unit = Seq();
        if ( !unit.DoMatch( forge))
            return false; 
        return true;
    }

    void    Dump( std::ostream &ostr) const
    {
        ostr << "Seq";
        return;
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
            forge->Pred< RExpAtom>()->m_ChSet = forge->m_ChSet;

            std::cout << forge->MatchStr() << "\n";
            return true;  }; } 

    auto        SeqListener(void) const {
        return [this]( auto forge) { 
            std::cout << forge->MatchStr() << "\n"; 
            auto            docWhorl = forge->Bottom< RExpDoc>();
            forge->Pred< RExpAtom>()->m_Id = forge->FetchId( docWhorl->m_Repos);
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
    void    Dump( std::ostream &ostr) const
    {
        ostr << "Atom";
        return;
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
        uint32_t		        m_Min;
        uint32_t		        m_Max;  
        bool                    m_Stingy;
        RExpCrate::Id           m_Id; 

        Whorl(void)
            :  m_Min( 0), m_Max( 0), m_Stingy( false) 
        {} 

    template < typename Parser>
        void Scavenge( Parser *parser)
        {
            auto    docWhorl = parser->Bottom< RExpDoc>(); 
        }

        bool            IsBasic( void) const 
        { 
            return ( m_Min == 0) && ( m_Max == 0); 
        }

        RExpCrate::Id           FetchId( RExpRepos *repos) 
        {
            if ( IsBasic())
                return m_Id;
            
            auto    synElem = new RepeatSynElem();
            synElem->m_Elem = m_Id; 
            synElem->m_Min = m_Min;
            synElem->m_Max = m_Max;
            return repos->Store( synElem);   
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
            return true;  }; }

    auto		CommaListener(void) const {
        return [](auto forge) {
            Whorl       *whorl = forge->Pred< RExpQuanta>();  
            whorl->m_Max = CV_UINT32_MAX; 
            return true;  }; }
 

    auto		QuestionListener(void) const {
        return []( auto forge) {
            Whorl       *whorl = forge->Pred< RExpQuanta>();
            whorl->m_Min = 0;
            whorl->m_Max = 1; 
            return true;  }; }

    auto		StarListener(void) const {
        return []( auto forge) {
            Whorl       *whorl = forge->Pred< RExpQuanta>();
            whorl->m_Min = 0;  
            whorl->m_Max = CV_UINT32_MAX;  
            return true;  }; }

    auto		PlusListener(void) const {
        return []( auto forge) {
            Whorl       *whorl = forge->Pred< RExpQuanta>();
            whorl->m_Min = 1;  
            whorl->m_Max = CV_UINT32_MAX;  
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

    auto        Spiner( void) const {
        return ( Char('{') >> ( !m_SpinMin[ MinSpinListener()] >> !(Char(',')[ CommaListener()] >> !(m_SpinMax[ MaxSpinListener()])))  >> Char('}')) |
                 Char('?')[ QuestionListener()] | Char('*')[ StarListener()] | Char('+')[ PlusListener()]; }

    auto		Quanta( void) const { 
        return m_Atom[ AtomListener()] >> !( Spiner() >> !Char('?')[ StingyListener()]) ; } 

template < typename Forge>
    bool    DoParse( Forge *forge) const
    {
        forge->Push();
        auto	unit = Quanta();
        if ( !unit.DoMatch( forge))
            return false; 
        return true;
    }

    void    Dump( std::ostream &ostr) const
    {
        ostr << "Quanta";
        return;
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

	auto          SeqListener(void) const {
		return [this]( auto forge) { 
            std::cout << forge->MatchStr() << "\n";
            auto        docWhorl = forge->Bottom< RExpDoc>();
 	        forge->Pred< RExpEntry>()->m_RExps.push_back( forge->FetchId( docWhorl->m_Repos));
			return true;  }; }

	auto           RExpression(void) const { return (+(RExpSeq()[ SeqListener()] - Char('/')))[ RExpressionListener()]; }
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
    void    Dump( std::ostream &ostr) const
    {
        ostr << "Entry";
        return;
    } 
};

//_____________________________________________________________________________________________________________________________

struct RExpDoc  : public Shard< RExpDoc>
{   
    struct XAct
    {
        RExpRepos       *m_Repos;
        RExpCrate::Id   m_Id;

        XAct( RExpRepos *repos) 
            : m_Repos( repos)
        {} 
    };
 
	struct Whorl
	{
        RExpRepos                       *m_Repos; 
        std::vector< RExpCrate::Id>     m_RExps;
        RExpCrate::Id                   m_Id;
        
        bool    PrimeIn( XAct *xact)
        {
            m_Repos = xact->m_Repos;
            return true;
        }
        
        bool    ExtractOut( XAct *xact)
        {
            xact->m_Id = m_Id;
            return true;
        }

        RExpCrate::Id           FetchId( RExpRepos *repos) 
        {
            auto    synElem = new AltSynElem();
            synElem->m_AltList = m_RExps; 
            return repos->Store( synElem);        
        }
	};

	RExpDoc( void) 
    {}  

    auto          RExpListener(void) const {
        return [this]( auto forge) {  
            auto            docWhorl = forge->Pred< RExpDoc>();
            docWhorl->m_RExps.push_back( forge->FetchId( docWhorl->m_Repos));
            return true;  }; }

    
	auto           DocumentOver( void) const { return [ this]( auto forge) { 
        auto            docWhorl = forge->Bottom< RExpDoc>(); 
        docWhorl->m_Id = docWhorl->FetchId( docWhorl->m_Repos);
		return true;  };  } 
	 
	auto           Document( void) const { return (+RExpEntry()[ RExpListener()] )[ DocumentOver()]; } 

template < typename Forge>
	bool    DoParse( Forge *forge) const
	{    
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
    void    Dump( std::ostream &ostr) const
    {
        return;
    } 
};

//_____________________________________________________________________________________________________________________________


inline auto	RExpAlt::Alt( void) const {
    RExpQuanta  quanta;
    return  (( quanta[ QuantaListener()] >>  *( OptBlankSpace() >> ( Char( '|')[ AltCharListener()] >>  quanta[ QuantaListener()]))  )); }

//_____________________________________________________________________________________________________________________________

};