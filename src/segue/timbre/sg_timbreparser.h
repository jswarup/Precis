//  sg_timbreparser.h ___________________________________________________________________________________________________________________
#pragma once

#include    "cove/barn/cv_cexpr.h"
#include    "cove/silo/cv_stack.h"
#include    "segue/timbre/sg_timbreshard.h"
  
namespace Sg_Timbre
{
   
//_____________________________________________________________________________________________________________________________ 

struct  StrInStream : public std::vector< char>
{ 
    uint32_t                    m_Cursor;
        
	StrInStream( void)
		: m_Cursor( 0)
	{}

	StrInStream( const std::string &str)
        : std::vector< char>( str.size() +1), m_Cursor( 0)
    {
        std::copy( str.begin(), str.end(), begin());
        SELF[ str.size()] = 0;
    } 

    bool                HasMore( void) { return m_Cursor < size(); }
    bool                Next( void) { return ++m_Cursor < size(); } 
    char                Curr( void) { return SELF[ m_Cursor]; }
    uint32_t            Marker( void) const { return m_Cursor; } 
    
    void                RollTo( uint32_t mark) { m_Cursor = mark; }
    uint32_t            SzFrom( uint32_t mark) { return m_Cursor -mark; }

    Cv_CStr             Region( uint32_t m1, uint32_t m2) { return Cv_CStr( &SELF[ m1], m2 -m1); }
}; 



//_____________________________________________________________________________________________________________________________ 

template < typename TP>
struct   Forge :  public Cv_StackVar< Forge< TP> >
{  
	typedef TP 						Parser; 
	typedef typename Parser::Mark	Mark; 

protected:
	Parser          *m_Parser; 
	Mark            m_Marker;
	bool			m_PushedFlg;
	bool            m_MatchFlg;

public:
	Forge( Parser *parser )
		:   m_Parser( parser), m_Marker( parser->Marker()), m_PushedFlg( false), m_MatchFlg( false)
	{
		
	}

	~Forge(void)
	{
		if (m_PushedFlg)
		{
			Forge      *popForge = m_Parser->PopForge();
			CV_ERROR_ASSERT(popForge == this)
		}
		if ( !m_MatchFlg) 
			m_Parser->RollTo( m_Marker); 
	}

	void		Push( void) { m_Parser->PushForge(this); m_PushedFlg = true; }

	Parser      *GetParser( void) const { return m_Parser; }

	bool        IsMatch( void) const { return m_MatchFlg; }

	Forge       *Parent( void) const { return  this->GetBelow(); }


template < typename TimbreShard>
    void ProcessMatch( TimbreShard *shard, int k) 
    {         
        m_MatchFlg = true;
        if ( GetParser()->LogStream() && Cv_TypeEngage::Dump( shard, *GetParser()->LogStream(), 0))
            *GetParser()->LogStream() << ':' << MatchStr() << '\n';
        return ;
    } 
 
	uint32_t    SzMatch( void) const { return m_Parser->SzFrom( m_Marker); }
	Cv_CStr     MatchStr( void) { return m_Parser->Region( m_Marker, m_Parser->Marker()); }

template < typename TimbreShard>
	auto		*Whorl(void) { return static_cast<ShardForge < TimbreShard, Parser> *>(this);  }



template < typename TimbreShard>
	auto		*Pred(void) { return m_PushedFlg ? Parent()->Whorl< TimbreShard>() : m_Parser->TopForge()->Whorl< TimbreShard>(); }


template < typename TimbreShard>
    auto		*Bottom(void) { return m_Parser->BottomForge()->Whorl< TimbreShard>(); }
};


//_____________________________________________________________________________________________________________________________ 

template < typename InStream>
class Parser 
{
public: 
    typedef uint32_t                    Mark;  
    typedef char                        Item; 
    typedef Sg_Timbre::SynElem          SynElem; 
	typedef Forge< Parser>				Forge;
    
protected:
    
    Cv_Fifo< Forge>        m_ForgeStack;
    InStream                *m_InStream; 
    std::ostream            *m_LogStream; 
    Mark                    m_LastMatch;

public:
    Parser( InStream *inStream) 
        : m_InStream( inStream), m_LogStream( NULL)
    {}

    ~Parser( void) { }

    std::ostream    *LogStream( void) { return m_LogStream; }
    void            SetLogStream( std::ostream *ostr) { m_LogStream = ostr; }
        
    void            PushForge( Forge *forge) { m_ForgeStack.PushFront( forge); }
    Forge           *PopForge( void)  { return m_ForgeStack.PopFront();  }
    
    Forge           *TopForge( void) { return m_ForgeStack.Top(); }
    Forge           *BottomForge( void) { return m_ForgeStack.Bottom(); } 
    bool            HasMore( void) { return m_InStream->HasMore(); }
    bool            Next( void) { return m_InStream->Next(); } 
    Item            Curr( void) { return m_InStream->Curr(); } 
    Mark            Marker( void) const { return m_InStream->Marker(); }
    
      
    void            RollTo( const Mark &mark) { m_InStream->RollTo( mark); }
    uint32_t        SzFrom( const Mark &mark) { return m_InStream->SzFrom( mark); }
    Cv_CStr         Region( const Mark &m1, const Mark &m2) { return m_InStream->Region( m1, m2); }
    

template < typename TimbreShard>
    auto		    *Bottom(void) { return BottomForge()->Whorl< TimbreShard>(); }

template < typename TimbreShard>
    auto		    *Top(void) { return TopForge()->Whorl< TimbreShard>(); }

template < typename Shard>    
    bool            Match( Shard *node)
    {
        Forge  forge( this);
       
        bool    res = node->DoMatch( &forge); 
        return res;
    } 

template < typename Shard, typename Data>    
    bool            Match( Shard *node, Data *data)
    { 
        ShardForge< Shard, typename Forge::Parser>      forge( this);
        forge.PrimeIn( data);
  
        bool        match = node->DoParse( &forge);
        if ( match)
            forge.ExtractOut( data);
        return match;
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
        strm << "R" << m_IPtr << " [ shape=diamond  label= <<FONT> \"" << m_Filt.ToString() << "\" <BR />"; 
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
    {}
    
    std::string             Name( void) { return std::string( "Str:" + m_Str); }
template < typename Forge>    
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::Parser      *parser = ctxt->GetParser();
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
        Cv_Aid::UpCase( &m_Str);
    }
     
template < typename Forge>    
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::Parser      *parser = ctxt->GetParser();
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
    {}

template < typename Forge>    
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::Parser      *parser = ctxt->GetParser(); 
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
    {}
        
template < typename Forge>  
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::Parser     *parser = ctxt->GetParser(); 
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
    {}

template < typename Forge>    
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::Parser      *parser = ctxt->GetParser(); 
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
    {}

template < typename Forge>    
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::Parser      *parser = ctxt->GetParser();         
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
        typename Forge::Parser     *parser = ctxt->GetParser();  
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
 