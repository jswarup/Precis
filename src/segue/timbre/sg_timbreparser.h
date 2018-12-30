//  sg_timbreparser.h ___________________________________________________________________________________________________________________
#pragma once

#include    "cove/barn/cv_cexpr.h"
#include    "cove/silo/cv_stack.h"
#include    "segue/timbre/sg_timbreshard.h"
  
namespace Sg_Timbre
{
   
//_____________________________________________________________________________________________________________________________ 

struct  StrInStream
{
    
    std::vector< char>          m_StrVec;
    uint32_t                    m_Cursor;
        
	StrInStream( const std::string &str)
        : m_StrVec( str.size() +1), m_Cursor( 0)
    {
        std::copy( str.begin(), str.end(), m_StrVec.begin());
        m_StrVec[ str.size()] = 0;
    }
    
    bool                HasMore( void) { return m_Cursor < m_StrVec.size(); }
    bool                Next( void) { return ++m_Cursor < m_StrVec.size(); } 
    char                Curr( void) { return m_StrVec[ m_Cursor]; }
    uint32_t            Marker( void) const { return m_Cursor; } 
    
    void                RollTo( uint32_t mark) { m_Cursor = mark; }
    uint32_t            SzFrom( uint32_t mark) { return m_Cursor -mark; }

    Cv_CStr             Region( uint32_t m1, uint32_t m2) { return Cv_CStr( &m_StrVec[ m1], m2 -m1); }
};

//_____________________________________________________________________________________________________________________________ 

template < typename InStream>
class Parser 
{
public: 
    typedef uint32_t                    Mark;  
    typedef char                        Item; 
    typedef Sg_Timbre::SynElem          SynElem; 
    
    struct   Forge :  public Cv_StackVar< Forge >
    {  
        typedef  Parser                         TParser;
    protected:
        Parser          *m_Parser; 
        Mark            m_Marker;
        bool            m_MatchFlg;

    public:
        Forge( Parser *parser )
            :   m_Parser( parser), m_Marker( parser->Marker()), m_MatchFlg( false)
        {
            m_Parser->PushForge( this);
        }

        ~Forge( void)
        {
            Forge      *popForge = m_Parser->PopForge();    
            if ( !m_MatchFlg) 
                m_Parser->RollTo( m_Marker); 
        }

        Parser      *GetParser( void) const { return m_Parser; }

        bool        IsMatch( void) const { return m_MatchFlg; }

        Forge       *Parent( void) const { return  this->GetBelow(); }

        void        NotifyChildMatch( Forge *child)
        {}

        void        ProcessMatch( void)
        {        
            m_MatchFlg = true;
            return;
        }
        
        uint32_t    SzMatch( void) const { return m_Parser->SzFrom( m_Marker); }
        Cv_CStr     MatchStr( void) { return m_Parser->Region( m_Marker, m_Parser->Marker()); }
        
    };
protected:
    
    Cv_Stack< Forge>        m_ForgeStack;
    InStream                *m_InStream; 
    Mark                    m_LastMatch;

public:
    Parser( InStream *inStream) 
        : m_InStream( inStream)
    {}

    ~Parser( void) { }

    void            PushForge( Forge *forge) { m_ForgeStack.Push( forge); }
    Forge           *PopForge( void)  { return m_ForgeStack.Pop();  }
    
    Forge           *TopForge( void) { return m_ForgeStack.Top(); }
    Forge           *BottomForge( void) { return m_ForgeStack.Bottom(); } 
    bool            HasMore( void) { return m_InStream->HasMore(); }
    bool            Next( void) { return m_InStream->Next(); } 
    Item            Curr( void) { return m_InStream->Curr(); } 
    Mark            Marker( void) const { return m_InStream->Marker(); }
    
      
    void            RollTo( const Mark &mark) { m_InStream->RollTo( mark); }
    uint32_t        SzFrom( const Mark &mark) { return m_InStream->SzFrom( mark); }
    Cv_CStr         Region( const Mark &m1, const Mark &m2) { return m_InStream->Region( m1, m2); }
    
template < typename Shard>    
    bool            Match( Shard *node)
    {
        Forge  forge( this);
        
        return node->DoMatch( &forge);
    } 
}; 
 

//_____________________________________________________________________________________________________________________________ 

struct     StrSynElem : public SynElem 
{
    std::string     m_Str;
    bool            m_CaselessFlg;


	std::string		GetName( void) const { return Cv_Aid::ToStr( "Str", GetId()); }

    StrSynElem( void)
        :   m_CaselessFlg( false)
    {}

    bool    WriteDot( Cv_DotStream &strm)  
    {
        strm << "R" << m_IPtr << " [ shape=diamond  label= <<FONT> #" << GetName() << " <BR />"; 
        strm << " </FONT>>];\n "; 
        return true;
    } 
};

//_____________________________________________________________________________________________________________________________ 

struct     CSetSynElem : public SynElem 
{
    Sg_ChSet     m_Filt;
	 
	std::string		GetName( void) const { return Cv_Aid::ToStr( "CSet", GetId()); } 

    bool    WriteDot( Cv_DotStream &strm)  
    {
        strm << "R" << m_IPtr << " [ shape=diamond  label= <<FONT> #" << GetName() << " <BR />"; 
        strm << " </FONT>>];\n "; 
        return true;
    }
 
};
//_____________________________________________________________________________________________________________________________ 

struct Str : public Shard< Str >
{     
    std::string     m_Str;
    
    Str( const std::string & s)
        :   m_Str( s)
    {
        SetName( std::string( "Str:" + s));
    }
    
template < typename Forge>    
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::TParser      *parser = ctxt->GetParser();
        const char                  *pCh = m_Str.c_str();
        while ( *pCh)
        {
            bool    match = parser->HasMore() && ( *pCh == parser->Curr());
            if ( ! match) 
                return false; 
            parser->Next();                       
            ++pCh;
        }
        return true;
    } 

template < typename Cnstr>
	auto        FetchElemId( Cnstr *cnstr)
	{  
		auto			*elem = new StrSynElem();            
		elem->m_Str = m_Str;  
		return cnstr->Store( elem);
	} 
};

//_____________________________________________________________________________________________________________________________ 

struct IStr : public Shard< IStr >
{     
    std::string     m_Str;
    
    IStr( const std::string & s)
        :   m_Str( s)
    {
        SetName( std::string( "IStr:" + s)); 
        Cv_Aid::UpCase( &m_Str);
    }
    
template < typename Forge>    
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::TParser      *parser = ctxt->GetParser();
        const char                  *pCh = m_Str.c_str();
        while ( *pCh)
        {
            bool    match = parser->HasMore() && ( *pCh ==  Cv_Aid::UpCase( parser->Curr()));
            if ( ! match) 
                return false; 
            parser->Next();                       
            ++pCh;
        }
        return true;
    }
	 

template < typename Cnstr>
	auto        FetchElemId( Cnstr *cnstr)
	{  
		auto			*elem = new StrSynElem();            
		elem->m_Str = m_Str; 
		elem->m_CaselessFlg = true; 
		return cnstr->Store( elem);
	} 
};

//_____________________________________________________________________________________________________________________________ 

struct Char : public Shard< Char >
{
    const char      m_Char;

    Char( const char ch)
        : m_Char( ch)
    {
        SetName( std::string( "Char:" + ch));
    }

template < typename Forge>    
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::TParser      *parser = ctxt->GetParser(); 
        bool    match = parser->HasMore() && ( parser->Curr() == m_Char);
        if ( !match)
            return false;
        parser->Next();
        return  true;;
    }   
	  

template < typename Cnstr>
	auto        FetchElemId( Cnstr *cnstr)
	{  
		auto			*elem = new StrSynElem();            
		elem->m_Str.push_back( m_Char); 
		return cnstr->Store( elem);
	} 
};

//_____________________________________________________________________________________________________________________________ 
     
struct CharRange  : public  Shard< CharRange >
{
	uint8_t		m_C1;
	uint8_t		m_C2;

public:
	CharRange( uint8_t c1, uint8_t c2) 
		: m_C1( c1), m_C2( c2)
    {
        this->SetName( std::string( "Range:" + m_C1 + '-' + m_C2)) ;
    }
        
template < typename Forge>  
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::TParser     *parser = ctxt->GetParser(); 
        bool                        match = parser->HasMore() && (( m_C1 <= parser->Curr()) && ( parser->Curr() < m_C2));
        if ( !match)
            return false;
        parser->Next();
        return  true;
    }  

template < typename Cnstr>
	auto        FetchElemId( Cnstr *cnstr)
	{  
		auto			*elem = new CSetSynElem();  
		return cnstr->Store( elem);
	} 
};

//_____________________________________________________________________________________________________________________________ 

struct EoS : public Shard< EoS >
{
    EoS( void)
    {
        SetName( "EoS");
    }

template < typename Forge>    
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::TParser      *parser = ctxt->GetParser(); 
        return  !parser->HasMore();
    }   
	 

template < typename Cnstr>
	auto        FetchElemId( Cnstr *cnstr)
	{  
		auto			*elem = new CSetSynElem(); 
		return cnstr->Store( elem);
	} 
};

//_____________________________________________________________________________________________________________________________ 

struct Any : public Shard< Any>
{
    Any( void)
    {
        SetName( "Any");
    }

template < typename Forge>    
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::TParser      *parser = ctxt->GetParser();         
        parser->Next();
        return  true;
    }   


template < typename Cnstr>
	auto        FetchElemId( Cnstr *cnstr)
	{  
		auto			*elem = new CSetSynElem(); 
		return cnstr->Store( elem);
	} ;
};

//_____________________________________________________________________________________________________________________________ 

struct CharSet : public Shard< CharSet >
{
     
    Sg_ChSet     m_Bits;

    CharSet( const std::string &set)
    { 
        SetName( std::string( "CharSet:" +set));
        const char      *p = set.c_str();
        while ( *p)
        {
            const char      *first = p;
            m_Bits.Set( *first, true);
            ++p;

            const char      *second = (p + 1);
            if ( *p && *second && ( *p == '-') )
            {
                for ( char c = *first; c <= *second; ++c)
                    m_Bits.Set( c, true);
                p += 2;
            }
        }
    }

    
    CharSet operator ~( void) const 
    {
        CharSet     neg( GetName());
        neg.m_Bits.Negate();
        return neg;
    }

template < typename Forge>    
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::TParser     *parser = ctxt->GetParser();  
        bool                        match = parser->HasMore() && m_Bits.Get( parser->Curr());
        if ( ! match)
            return false;
        parser->Next();
        return true;
    } 

template < typename Cnstr>
	auto        FetchElemId( Cnstr *cnstr)
	{  
		auto			*elem = new CSetSynElem();  
		elem->m_Filt = m_Bits;
		return cnstr->Store( elem);
	} 
};
 
//_____________________________________________________________________________________________________________________________ 

typedef Cv_Crate< StrSynElem, CSetSynElem,  SynCrate>   SynParserCrate; 
 
};
//_____________________________________________________________________________________________________________________________ 
 