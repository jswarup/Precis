//  sg_timbreshard.h ___________________________________________________________________________________________________________________
#pragma once

#include    "cove/barn/cv_cexpr.h"
#include 	"cove/barn/cv_aid.h"
#include    "segue/timbre/sg_syntaxtree.h"

//_____________________________________________________________________________________________________________________________  

namespace Sg_Timbre
{
    
//_____________________________________________________________________________________________________________________________  

    
template <typename Left, typename Right>
struct Seq;

template <typename TShard, typename Actor>
struct Action;

template <typename Child, uint32_t Min, uint32_t Max>
struct Repeat; 

template <typename Left, typename Right>
struct Alt;

template <typename Left, typename Right>
struct Diff;

template < typename Right>
struct LexemeShard;

template < typename Right>
struct RefShard;

template <const char C1, const char C2>
struct RangeShard;

//_____________________________________________________________________________________________________________________________  

template < typename TimbreShard, typename Parser, typename = void>
struct ShardForge : public  Parser::Forge
{   
	ShardForge( Parser *parser)
		: Parser::Forge(parser) 
	{}

};

template <typename TimbreShard, typename Parser>
struct ShardForge< TimbreShard, Parser, typename Cv_TypeEngage::Exist< typename TimbreShard::Whorl>::Note> : public  Parser::Forge, public  TimbreShard::Whorl
{
    typedef typename TimbreShard::Whorl  Whorl;
    
	ShardForge( Parser *parser)
		: Parser::Forge(parser)
	{} 
}; 

//_____________________________________________________________________________________________________________________________  

template <typename TimbreShard, typename Parser,  typename Data>
struct DataForge : public Parser::Forge
{ 
    Data        *m_Data;

    DataForge( Parser *parser, Data *data)
        : Parser::Forge( parser), m_Data( data)
    {}
}; 

//_____________________________________________________________________________________________________________________________  
 
template < typename TShard>
struct Shard 
{ 
    typedef  TShard          GrammarShard; 
    
    std::string             m_Name; 

public:
	Shard( void)  
	{}

    GrammarShard            *GetShard( void)  { return static_cast< GrammarShard *>( this); }
    const GrammarShard      *GetShard( void) const  { return static_cast< const GrammarShard *>( this); }

    std::string             Name( void) const { return std::string(); } 
    std::string             GetName( void) const { return GetShard()->Name(); }
 	
template <typename  Forge>   
    bool    DoParse( Forge *ctxt) const
    {
        return ctxt->IsMatch();
    }

template <typename ParentForge>
    bool DoMatch( ParentForge *ctxt) const
    {
        ShardForge< GrammarShard, typename ParentForge::Parser>      forge( ctxt->GetParser());

        bool        match = this->GetShard()->DoParse( &forge);
        if ( match)   
            forge.ProcessMatch( this->GetShard(), 0);  
        
        return match;
    }  

template < typename Right>
    Seq< GrammarShard, Right>                   operator>>( const Right & r) const;

template <typename Actor>
    Action< GrammarShard, Actor >               operator[]( const Actor &actor) const;    

    auto										operator!( void) const;
    auto                                        operator!( void);
    
    auto                                        operator+( void) const;
    auto                                        operator+( void);
    
    auto                                        operator*( void) const;
    auto                                        operator*( void);
    
template < typename Right>
    Alt< GrammarShard, Right>                   operator|( const Right & r) const;

template < typename Right>
    Diff< GrammarShard, Right>                  operator-( const Right & r) const; 

template < typename Right>
    LexemeShard< Right>                         Lexeme( const Right &p) const { return LexemeShard< Right>( p); }
	  
template < typename Right>
	RefShard< Right>							Ref( const Right &p) const { return RefShard< Right>( p); } 
};

//_____________________________________________________________________________________________________________________________ 

struct Error : public  Shard< Error >
{ 
    std::string     m_ErrStr;

public:
    Error( const std::string &str) 
        :  m_ErrStr( str)
    {}
        
template <typename ParentForge>
    bool DoMatch( ParentForge *ctxt) const
    { 
        return false;  
    } 
	  
template < typename Cnstr>
	auto        FetchElemId( Cnstr *cnstr)
	{  
		auto elem = new Sg_Timbre::ErrorSynElem();
		elem->m_ErrStr = m_ErrStr; 
		return cnstr->Store( elem);
	} 
};


//_____________________________________________________________________________________________________________________________ 

template <typename TShard>
struct RefShard  : public  Shard< RefShard< TShard> >
{
	typedef typename TShard::GrammarShard     Target;

	Target          *m_Shard; 

public: 
	RefShard( const TShard &node)
		: m_Shard( const_cast< Target *>( node.GetShard()))
	{} 

template <typename ParentForge>
	bool DoMatch( ParentForge *ctxt) const
	{
		ShardForge< TShard, typename ParentForge::Parser>       forge( ctxt->GetParser());
 

		bool        match = m_Shard->DoMatch( &forge);
		if ( match)   
			forge.ProcessMatch( m_Shard, 0);   

		return match;
	} 

template < typename Cnstr>
	auto        FetchElemId( Cnstr *cnstr)
	{  
		auto			*elem = new RefSynElem();
		elem->m_Elem = cnstr->FetchElemId( m_Shard); 
		return cnstr->Store( elem);
	} 
};

//_____________________________________________________________________________________________________________________________ 

template <typename TShard, typename Actor>
struct Action  : public  Shard< Action< TShard, Actor> >
{
    typedef typename TShard::GrammarShard     Target;
    
    Target          m_Shard;
    Actor           m_Actor;

public:
    
   Action( const TShard &node, const Actor & right)
        : m_Shard( *node.GetShard()),  m_Actor( right)
    { }
 
 
template <typename ParentForge>
    bool DoMatch( ParentForge *ctxt) const
    {
        ShardForge< TShard, typename ParentForge::Parser>       forge( ctxt->GetParser()); 


        bool        match = m_Shard.DoParse( &forge);
        if ( match)  
        { 
            match = m_Actor( &forge);
            if ( match)
                forge.ProcessMatch( &m_Shard, 0);
        }
        
        return match;
    } 

template < typename Cnstr>
	auto        FetchElemId( Cnstr *cnstr)
	{  
		auto			*elem = new ActionSynElem();
		elem->m_Elem = cnstr->FetchElemId( &m_Shard); 
		cnstr->Repos()->ToVar( elem->m_Elem).GetEntry()->m_LockFlg = true;
		return cnstr->Store( elem);
	} 
};


//_____________________________________________________________________________________________________________________________ 
///  Lexeme class defines a new grammar terminator. 

template < typename TShard>
struct LexemeShard : public  Shard< LexemeShard< TShard> >
{
    typedef typename TShard::GrammarShard     Target;

    Target          m_LexShard;

public:
    LexemeShard( const TShard &node)
        : m_LexShard( *node.GetShard()) 
    {}

template <typename ParentForge>
    bool DoMatch( ParentForge *ctxt) const
    { 
        return m_LexShard.DoMatch( ctxt);  
    } 
	 

template < typename Cnstr>
	auto        FetchElemId( Cnstr *cnstr)
	{  
		auto	elem = new LexemeSynElem();
		elem->m_Elem = cnstr->FetchElemId( &m_LexShard);  
		return cnstr->Store( elem);
	} 
};
 
//_____________________________________________________________________________________________________________________________ 
// sequence

template <typename Left, typename Right>
struct Seq : public  Shard< Seq< Left, Right> >
{
    typedef typename    Left::GrammarShard    TargetLeft;
    typedef typename    Right::GrammarShard   TargetRight;
    
    TargetLeft          m_Left;
    TargetRight         m_Right;
    
    Seq( const Left & left, const Right & right)
        : m_Left( *left.GetShard()),  m_Right( *right.GetShard())
    { }

template < typename Forge>
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::Parser	*parser = ctxt->GetParser(); 
        bool                    match = false; 
        
        if ( m_Left.DoMatch( ctxt)) 
            match = m_Right.DoMatch( ctxt);   
        return match;
    }

template < typename Cnstr>
	auto        FetchElemId( Cnstr *cnstr)
	{  
        typedef typename Cnstr::Crate Crate;
        
		auto		seqType = Crate::TypeOf<SeqSynElem>();	
		auto		leftId = cnstr->FetchElemId( &m_Left);
		auto		rightId = cnstr->FetchElemId( &m_Right);
		
		bool		lSeqFlg = ( leftId.GetType() == seqType);
		bool		rSeqFlg = ( rightId.GetType() == seqType);
		if ( lSeqFlg && !rSeqFlg)
		{
			SeqSynElem	*lSeq = static_cast< SeqSynElem*>( cnstr->Repos()->ToVar( leftId).GetEntry());
			if ( !lSeq->IsLocked())
			{
				lSeq->m_SeqList.push_back( rightId);
				return leftId;
			}
		}
		if ( !lSeqFlg && rSeqFlg)
		{
			SeqSynElem	*rSeq = static_cast< SeqSynElem*>( cnstr->Repos()->ToVar( rightId).GetEntry());
			if ( !rSeq->IsLocked())
			{
				rSeq->m_SeqList.insert( rSeq->m_SeqList.begin(), leftId);
				return rightId;
			}
		}
		if ( lSeqFlg && rSeqFlg)
		{
			SeqSynElem	*lSeq = static_cast< SeqSynElem*>( cnstr->Repos()->ToVar( leftId).GetEntry());
			SeqSynElem	*rSeq = static_cast< SeqSynElem*>( cnstr->Repos()->ToVar( rightId).GetEntry());
			if ( !rSeq->IsLocked() && !rSeq->IsLocked())
			{
				lSeq->m_SeqList.reserve( lSeq->m_SeqList.size() + rSeq->m_SeqList.size() ); 
				lSeq->m_SeqList.insert( lSeq->m_SeqList.end(), rSeq->m_SeqList.begin(), rSeq->m_SeqList.end() );
				cnstr->Repos()->Destroy( rightId.GetId());
				return leftId;
			}
		}
		auto			*elem = new SeqSynElem();            
		elem->m_SeqList.push_back( leftId); 
		elem->m_SeqList.push_back( rightId); 
		return cnstr->Store( elem);
	} 

    void    Dump( std::ostream &ostr)
    {
        return;
    } 
};

//_____________________________________________________________________________________________________________________________ 


template <typename Child, uint32_t Min, uint32_t Max>
struct Repeat  : public  Shard< Repeat< Child, Min, Max> >
{ 
    typedef typename Child::GrammarShard     Target;
    
    Target                                  m_Target;
    
    Repeat( const Child & target)
        : m_Target( target)
    { }
 

template < typename Forge>
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::Parser		*parser = ctxt->GetParser();  
        uint32_t                    count = 0;
        while ( count < Min)
        {
            if ( !parser->IsCurValid() || !m_Target.DoMatch( ctxt) || !ctxt->SzMatch())
                return false;
            ++count; 
        }

        while ( count < Max)
        {
            if ( !parser->IsCurValid() || !m_Target.DoMatch( ctxt) || !ctxt->SzMatch())
                return true; 
            ++count;
        }
         
        return true;
    } 

template < typename Cnstr>
	auto        FetchElemId( Cnstr *cnstr)
	{  
		auto			*elem = new RepeatSynElem();            
		elem->m_Elem = cnstr->FetchElemId( &m_Target);
		elem->m_Min = Min;
		elem->m_Max = Max; 
		return cnstr->Store( elem);
	} 
};

//_____________________________________________________________________________________________________________________________ 
// Alternative, left First match

template <typename Left, typename Right>
struct Alt : public Shard< Alt< Left, Right> >
{
    typedef typename    Left::GrammarShard    TargetLeft;
    typedef typename    Right::GrammarShard   TargetRight;
    
    TargetLeft      m_Left;
    TargetRight     m_Right; 

    Alt( const Left &left, const Right &right) 
        : m_Left( left), m_Right( right)
    {}

template < typename Forge>
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::Parser     *parser = ctxt->GetParser(); 
         
        bool    match = m_Left.DoMatch( ctxt);
        if ( !match) 
            match = m_Right.DoMatch( ctxt);
        return match;
    } 

template < typename Cnstr>  
	auto        FetchElemId( Cnstr *cnstr)
	{  
		auto		altType = Cnstr::Crate::TypeOf<AltSynElem>();	
		auto		leftId = cnstr->FetchElemId( &m_Left);
		auto		rightId = cnstr->FetchElemId( &m_Right);

		bool		lAltFlg = ( leftId.GetType() == altType);
		bool		rAltFlg = ( rightId.GetType() == altType);
		if ( lAltFlg && !rAltFlg)
		{
			AltSynElem	*lSeq = static_cast< AltSynElem*>( cnstr->Repos()->ToVar( leftId).GetEntry());
			if ( !lSeq->IsLocked())
			{
				lSeq->m_AltList.push_back( rightId);
				return leftId;
			}
		}
		if ( !lAltFlg && rAltFlg)
		{
			AltSynElem	*rSeq = static_cast< AltSynElem*>( cnstr->Repos()->ToVar( rightId).GetEntry());
			if ( !rSeq->IsLocked())
			{
				rSeq->m_AltList.insert( rSeq->m_AltList.begin(), leftId);
				return rightId;
			}
		}
		if ( lAltFlg && rAltFlg)
		{
			AltSynElem	*lSeq = static_cast< AltSynElem*>( cnstr->Repos()->ToVar( leftId).GetEntry());
			AltSynElem	*rSeq = static_cast< AltSynElem*>( cnstr->Repos()->ToVar( rightId).GetEntry());
			if ( !rSeq->IsLocked() && !rSeq->IsLocked())
			{
				lSeq->m_AltList.reserve( lSeq->m_AltList.size() + rSeq->m_AltList.size() ); 
				lSeq->m_AltList.insert( lSeq->m_AltList.end(), rSeq->m_AltList.begin(), rSeq->m_AltList.end() );
				cnstr->Repos()->Destroy( rightId.GetId());
				return leftId;
			}
		}
		auto			*elem = new AltSynElem();            
		elem->m_AltList.push_back( leftId); 
		elem->m_AltList.push_back( rightId); 
		return cnstr->Store( elem);
	} 
};

//_____________________________________________________________________________________________________________________________ 

template <typename Left, typename Right>
struct Diff : public Shard< Diff< Left, Right> >
{
    typedef typename    Left::GrammarShard    TargetLeft;
    typedef typename    Right::GrammarShard   TargetRight;

    TargetLeft      m_Left;
    TargetRight     m_Right; 

    Diff( const Left &left, const Right &right) 
        : m_Left( left), m_Right( right)
    {}

    template < typename Forge>
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::Parser     *parser = ctxt->GetParser(); 

        bool    match = m_Right.DoMatch( ctxt);
        if ( !match) 
            return m_Left.DoMatch( ctxt);
        return false;
    }
	 

template < typename Cnstr>
	auto        FetchElemId( Cnstr *cnstr)
	{  
		auto			*elem = new AltSynElem();            
		elem->m_AltList.push_back( cnstr->FetchElemId( &m_Left)); 
		elem->m_AltList.push_back( cnstr->FetchElemId( &m_Right)); 
		return cnstr->Store( elem);
	} 
};

//_____________________________________________________________________________________________________________________________ 

template <typename TShard>
template <typename Actor>
Action< TShard, Actor>               Shard< TShard>::operator[ ] ( const Actor &actor) const
{
    return Action<TShard, Actor>( * (const TShard *) this, actor);
}

//_____________________________________________________________________________________________________________________________

template <typename TShard>
template <typename Right>
Seq< TShard, Right>                  Shard< TShard>::operator>>( const Right &r) const
{
    return Seq< TShard, Right>(* (const TShard *) this, r);
}

//_____________________________________________________________________________________________________________________________

template <typename TShard>
auto               Shard< TShard>::operator!( void) const
{
    return Repeat< TShard, 0, 1>(* (const TShard *) this);
}

template <typename TShard>
auto               Shard< TShard>::operator!( void) 
{
    return Repeat< TShard, 0, 1>(* ( TShard *) this);
}

//_____________________________________________________________________________________________________________________________

template <typename TShard>
auto                                Shard< TShard>::operator+( void) const
{
    return Repeat< TShard, 1, CV_UINT32_MAX>(* (const TShard *) this);
}

template <typename TShard>
auto                                Shard< TShard>::operator+( void) 
{
    return Repeat< TShard, 1, CV_UINT32_MAX>(* ( TShard *) this);
}

//_____________________________________________________________________________________________________________________________

template <typename TShard>
auto                                Shard< TShard>::operator*( void) const
{
    return Repeat< TShard, 0, CV_UINT32_MAX>(* (const TShard *) this);
}

template <typename TShard>
auto                                Shard< TShard>::operator*( void)
{
    return Repeat< TShard, 0, CV_UINT32_MAX>( * ( TShard *) this);
}
 
//_____________________________________________________________________________________________________________________________ 

template <typename TShard>
template <typename Right>
Alt< TShard, Right>                  Shard< TShard>::operator|( const Right & r) const
{
    return Alt< TShard, Right>(* (const TShard *) this, r);
}


template <typename TShard>
template <typename Right>
Diff< TShard, Right>                  Shard< TShard>::operator-( const Right & r) const
{
    return Diff< TShard, Right>(* (const TShard *) this, r);
}

//_____________________________________________________________________________________________________________________________ 
 
};
