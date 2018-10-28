//  sg_timbrenode.h ___________________________________________________________________________________________________________________
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

template <typename TNode, typename Actor>
struct Action;

template <typename Child, uint32_t Min, uint32_t Max>
struct Repeat; 

template <typename Left, typename Right>
struct Alt;

template <typename Left, typename Right>
struct Diff;

template < typename Right>
struct LexemeNode;

template <const char C1, const char C2>
struct RangeNode;

//_____________________________________________________________________________________________________________________________  

template < typename TimbreNode, typename Parser, typename = void>
struct ForgeType 
{ 
    typedef typename Parser::Forge          Forge;
    typedef typename Parser::SynElem        SynElem;
};

template <typename TimbreNode, typename Parser>
struct ForgeType< TimbreNode, Parser, typename Cv_TypeEngage::Exist< typename TimbreNode::Forge>::Note> 
{ 
    typedef typename TimbreNode::Forge      Forge;
    typedef typename TimbreNode::SynElem    SynElem;
}; 

 

//_____________________________________________________________________________________________________________________________  
 
template < typename TNode>
struct Node 
{ 
    typedef  TNode          GrammarNode;
    
    std::string             m_Name;

    GrammarNode             *GetNode( void)  { return static_cast< GrammarNode *>( this); }
    const GrammarNode       *GetNode( void) const  { return static_cast< const GrammarNode *>( this); }
 

    const std::string       &GetName( void) const { return m_Name; }
    void                    SetName( const std::string & n) { m_Name = n; } 
    
template <typename  Forge>   
    bool    DoParse( Forge *ctxt) const
    {
        return ctxt->IsMatch();
    }

template <typename ParentForge>
    bool DoMatch( ParentForge *ctxt) const
    {
        typename ForgeType< GrammarNode, typename ParentForge::TParser>::Forge      forge( ctxt->GetParser());
        
        bool        match = this->GetNode()->DoParse( &forge);
        if ( match)  
        {
            forge.ProcessMatch(); 
            ctxt->NotifyChildMatch( &forge);
        }
        
        return match;
    }  

template < typename Right>
    Seq< GrammarNode, Right>                    operator>>( const Right & r) const;

template <typename Actor>
    Action< GrammarNode, Actor >                operator[ ] ( const Actor &actor) const;     
    
    auto                                        operator!( void) const;
    auto                                        operator!( void);
    
    auto                                        operator+( void) const;
    auto                                        operator+( void);
    
    auto                                        operator*( void) const;
    auto                                        operator*( void);
    
template < typename Right>
    Alt< GrammarNode, Right>                    operator|( const Right & r) const;

template < typename Right>
    Diff< GrammarNode, Right>                   operator-( const Right & r) const; 

template < typename Right>
    LexemeNode< Right>                          Lexeme( const Right &p) const { return LexemeNode< Right>( p); }

template <const char C1, const char C2>
    RangeNode< C1, C2>                          Range( void) const { return RangeNode< C1, C2>(); }
};

//_____________________________________________________________________________________________________________________________ 

struct Error : public  Node< Error >
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
     
    struct     SynElem : public Sg_Timbre::SynElem
    {
        std::string     m_ErrStr;
    };

template < typename Cnstr>
    auto        Setup( Cnstr *cnstr)
    {  
        auto    synItem = new SynElem{  m_ErrStr};         
        cnstr->m_Crate->Store( synItem);
        return synItem;
    } 
};

//_____________________________________________________________________________________________________________________________ 

template <typename TNode, typename Actor>
struct Action  : public  Node< Action< TNode, Actor> >
{
    typedef typename TNode::GrammarNode     Target;
    
    Target          m_Node;
    Actor           m_Actor;

public:
    
   Action( const TNode &node, const Actor & right)
        : m_Node( *node.GetNode()),  m_Actor( right)
    { }
 
 
template <typename ParentForge>
    bool DoMatch( ParentForge *ctxt) const
    {
        typename ForgeType< TNode, typename ParentForge::TParser>::Forge      forge( ctxt->GetParser());
        
        bool        match = m_Node.DoMatch( &forge);
        if ( match)  
        {
            forge.ProcessMatch(); 
            ctxt->NotifyChildMatch( &forge);
            match = m_Actor( forge);
        }
        
        return match;
    }
    
    struct     SynElem : public ActionSynElem
    { 
    };

template < typename Cnstr>
    auto        Setup( Cnstr *cnstr)
    {
        auto    synItem = new SynElem();
        synItem->m_Elem = cnstr->FetchSynTree( &m_Node);        
        cnstr->m_Crate->Store( synItem); 
        return synItem;
    } 
};


//_____________________________________________________________________________________________________________________________ 
///  Lexeme class defines a new grammar terminator. 

template < typename TNode>
struct LexemeNode : public  Node< LexemeNode< TNode> >
{
    typedef typename TNode::GrammarNode     Target;

    Target          m_LexNode;

public:
    LexemeNode( const TNode &node)
        : m_LexNode( *node.GetNode()) 
    {}

template <typename ParentForge>
    bool DoMatch( ParentForge *ctxt) const
    { 
        return m_LexNode.DoMatch( ctxt);  
    } 

    struct     SynElem : public LexemeSynElem
    {
        typedef typename Target::SynElem       TargetElem;
    };
template < typename Cnstr>
    auto        Setup( Cnstr *cnstr)
    { 
        auto    synItem = new SynElem(); 
        synItem->m_Elem = cnstr->FetchSynTree( &m_LexNode);         
        cnstr->m_Crate->Store( synItem);
        return synItem;
    } 
};
 
//_____________________________________________________________________________________________________________________________ 
// sequence

template <typename Left, typename Right>
struct Seq : public  Node< Seq< Left, Right> >
{
    typedef typename    Left::GrammarNode    TargetLeft;
    typedef typename    Right::GrammarNode   TargetRight;
    
    TargetLeft          m_Left;
    TargetRight         m_Right;
    
    Seq( const Left & left, const Right & right)
        : m_Left( *left.GetNode()),  m_Right( *right.GetNode())
    { }

template < typename Forge>
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::TParser  *parser = ctxt->GetParser(); 
        bool                    match = false; 
        
        if ( m_Left.DoMatch( ctxt)) 
            match = m_Right.DoMatch( ctxt);   
        return match;
    }

    struct     SynElem : public SeqSynElem
    {
    };
template < typename Cnstr>
    auto        Setup(  Cnstr *cnstr)
    {
        auto    synItem = new SynElem();
        synItem->m_SeqList.push_back( cnstr->FetchSynTree( &m_Left)); 
        synItem->m_SeqList.push_back( cnstr->FetchSynTree( &m_Right));        
        cnstr->m_Crate->Store( synItem);
        return synItem;
    } 
};

//_____________________________________________________________________________________________________________________________ 


template <typename Child, uint32_t Min, uint32_t Max>
struct Repeat  : public  Node< Repeat< Child, Min, Max> >
{ 
    typedef typename Child::GrammarNode     Target;
    
    Target                                  m_Target;
    
    Repeat( const Child & target)
        : m_Target( target)
    { }
 

template < typename Forge>
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::TParser     *parser = ctxt->GetParser(); 

        if ( !parser->HasMore( Min ? Min : 1))
            return false;
 
        uint32_t                    count = 0;
        while ( count < Min)
        {
            if ( !parser->HasMore( 1) || !m_Target.DoMatch( ctxt) || !ctxt->SzMatch())
                return false;
            ++count; 
        }

        while ( count < Max)
        {
            if ( !parser->HasMore( 1) || !m_Target.DoMatch( ctxt) || !ctxt->SzMatch())
                return true; 
            ++count;
        }
         
        return true;
    }

    struct     SynElem : public RepeatSynElem
    {  
    };        

template < typename Cnstr>
    auto        Setup( Cnstr *cnstr)
    {   
        auto    synItem = new SynElem();
        synItem->m_Elem = cnstr->FetchSynTree( &m_Target);
        synItem->m_Min = Min; 
        synItem->m_Max = Max;        
        cnstr->m_Crate->Store( synItem);
        return synItem;
    } 
};

//_____________________________________________________________________________________________________________________________ 
// Alternative, left First match

template <typename Left, typename Right>
struct Alt : public Node< Alt< Left, Right> >
{
    typedef typename    Left::GrammarNode    TargetLeft;
    typedef typename    Right::GrammarNode   TargetRight;
    
    TargetLeft      m_Left;
    TargetRight     m_Right; 

    Alt( const Left &left, const Right &right) 
        : m_Left( left), m_Right( right)
    {}

template < typename Forge>
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::TParser     *parser = ctxt->GetParser(); 
         
        bool    match = m_Left.DoMatch( ctxt);
        if ( !match) 
            match = m_Right.DoMatch( ctxt);
        return match;
    }

    struct     SynElem : public AltSynElem
    {
    };

template < typename Cnstr>
    auto        Setup(  Cnstr *cnstr)
    { 
        auto    synItem = new SynElem();
        synItem->m_AltList.push_back( cnstr->FetchSynTree( &m_Left)); 
        synItem->m_AltList.push_back( cnstr->FetchSynTree( &m_Right));        
        cnstr->m_Crate->Store( synItem);
        return synItem;  
    } 
     
};

//_____________________________________________________________________________________________________________________________ 

template <typename Left, typename Right>
struct Diff : public Node< Diff< Left, Right> >
{
    typedef typename    Left::GrammarNode    TargetLeft;
    typedef typename    Right::GrammarNode   TargetRight;

    TargetLeft      m_Left;
    TargetRight     m_Right; 

    Diff( const Left &left, const Right &right) 
        : m_Left( left), m_Right( right)
    {}

    template < typename Forge>
    bool    DoParse( Forge *ctxt) const
    {   
        typename Forge::TParser     *parser = ctxt->GetParser(); 

        bool    match = m_Left.DoMatch( ctxt);
        if ( !match) 
            match = m_Right.DoMatch( ctxt);
        return match;
    }

    struct     SynElem : public AltSynElem
    {
    };

template < typename Cnstr>
    auto        Setup( Cnstr *cnstr)
    {
        auto    synItem = new SynElem();
        synItem->m_AltList.push_back( cnstr->FetchSynTree( &m_Left)); 
        synItem->m_AltList.push_back( cnstr->FetchSynTree( &m_Right));        
        cnstr->m_Crate->Store( synItem);
        return synItem;  
    } 

};

//_____________________________________________________________________________________________________________________________ 

template <typename TNode>
template <typename Actor>
Action< TNode, Actor>               Node< TNode>::operator[ ] ( const Actor &actor) const
{
    return Action<TNode, Actor>( * (const TNode *) this, actor);
}

//_____________________________________________________________________________________________________________________________

template <typename TNode>
template <typename Right>
Seq< TNode, Right>                  Node< TNode>::operator>>( const Right &r) const
{
    return Seq< TNode, Right>(* (const TNode *) this, r);
}

//_____________________________________________________________________________________________________________________________

template <typename TNode>
auto               Node< TNode>::operator!( void) const
{
    return Repeat< TNode, 0, 1>(* (const TNode *) this);
}

template <typename TNode>
auto               Node< TNode>::operator!( void) 
{
    return Repeat< TNode, 0, 1>(* ( TNode *) this);
}

//_____________________________________________________________________________________________________________________________

template <typename TNode>
auto                                Node< TNode>::operator+( void) const
{
    return Repeat< TNode, 1, CV_UINT32_MAX>(* (const TNode *) this);
}

template <typename TNode>
auto                                Node< TNode>::operator+( void) 
{
    return Repeat< TNode, 1, CV_UINT32_MAX>(* ( TNode *) this);
}

//_____________________________________________________________________________________________________________________________

template <typename TNode>
auto                                Node< TNode>::operator*( void) const
{
    return Repeat< TNode, 0, CV_UINT32_MAX>(* (const TNode *) this);
}

template <typename TNode>
auto                                Node< TNode>::operator*( void)
{
    return Repeat< TNode, 0, CV_UINT32_MAX>( * ( TNode *) this);
}
 
//_____________________________________________________________________________________________________________________________ 

template <typename TNode>
template <typename Right>
Alt< TNode, Right>                  Node< TNode>::operator|( const Right & r) const
{
    return Alt< TNode, Right>(* (const TNode *) this, r);
}


template <typename TNode>
template <typename Right>
Diff< TNode, Right>                  Node< TNode>::operator-( const Right & r) const
{
    return Diff< TNode, Right>(* (const TNode *) this, r);
}

//_____________________________________________________________________________________________________________________________ 
 
};
