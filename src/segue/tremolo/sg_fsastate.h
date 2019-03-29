//  sg_fsastate.h _______________________________________________________________________________________________________________
#pragma once

#include    "cove/silo/cv_array.h"
#include    "cove/silo/cv_stack.h"
#include    "cove/silo/cv_repos.h"
#include 	"cove/barn/cv_ptrslot.h" 
#include 	"cove/barn/cv_compare.h" 
#include    "segue/tremolo/sg_filter.h"
#include    "cove/silo/cv_craterepos.h"
#include    "segue/timbre/sg_distrib.h"  

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{

struct    FsaRepos;
struct    FsaState;
struct    FsaElem;
struct    FsaSupState;
struct    FsaDfaState;
struct    FsaDfaCnstr;
struct    FsaDfaStateMap;
struct    FsaDfaStateMapCltn;

typedef Cv_Crate< FsaDfaState, FsaSupState, FsaElem, FsaState>              FsaCrate;  

//_____________________________________________________________________________________________________________________________ 

struct  Action
{
    std::vector< uint64_t>        m_Values;

    Action( void)
    {}
    

    Cv_CArr< uint64_t>      Tokens( void) { return m_Values.size() ? Cv_CArr< uint64_t>( &m_Values[ 0], uint32_t( m_Values.size())) : Cv_CArr< uint64_t>(); } 

    void         Push( uint64_t v) { m_Values.push_back( v); }
    void         Push( const Cv_CArr< uint64_t> &tokens)
    {   
        uint32_t        oldSz = uint32_t( m_Values.size());
        m_Values.resize( oldSz + tokens.Size());
        std::copy( tokens.Begin(), tokens.End(), &m_Values[ oldSz]);
    }

    int32_t         Compare( const Action &x2) const 
    {
        if ( m_Values.size() != x2.m_Values.size())
            return m_Values.size() < x2.m_Values.size() ? 1 : -1;
        const uint64_t       *arr1 = &m_Values[ 0];
        const uint64_t       *arr2 = &x2.m_Values[ 0];
        for ( uint32_t i = 0; i < m_Values.size(); ++i)
            if ( arr1[ i] != arr2[ i])
                return arr1[ i] < arr2[ i] ? 1 : -1;
        return 0;
    }
};

//_____________________________________________________________________________________________________________________________ 

struct FsaState  : public Cv_CrateEntry, public Cv_Shared
{
    typedef  Id                 FsaId;

    typedef FilterRepos::Id     FiltId;
    typedef FilterCrate::Var    FiltVar;
    
public:
    virtual     ~FsaState( void){}

    Cv_CArr< uint64_t>  Tokens( void) { return Cv_CArr< uint64_t>(); } 

    Cv_CArr< FiltId>    Filters( void) { return Cv_CArr< FiltId>(); }
    Cv_CArr< FsaId>     Dests( void) { return Cv_CArr< FsaId>(); }
    Cv_CArr< FsaId>     SubStates( void) { return Cv_CArr< FsaId>(); } 

    bool                WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm) { return false; }
};


//_____________________________________________________________________________________________________________________________ 

struct  FsaRepos  : public Cv_CrateRepos< FsaCrate>
{   
    typedef  Id                 FsaId;
    FsaId                       m_RootId;
    FilterRepos                 m_FilterRepos; 

    bool                WriteDot( Cv_DotStream &strm);
    bool                DumpDot( const char *path);
};

//_____________________________________________________________________________________________________________________________ 

struct  FsaElemRepos  : public FsaRepos
{    
    std::vector< uint32_t>      m_RuleIdSzList;

    uint32_t        RuleIdFromState( uint32_t k) const 
    {
        auto it = std::lower_bound( m_RuleIdSzList.begin(), m_RuleIdSzList.end(), k);
        CV_ERROR_ASSERT( it != m_RuleIdSzList.end());
        return *it;
    }
 
};

//_____________________________________________________________________________________________________________________________ 

struct FsaSupState  : public FsaState
{   
    Cv_Slot< FsaDfaStateMap>        m_DfaStateMap;
    std::vector< FsaId>             m_SubStates;  
    Action                          *m_Action; 

    FsaSupState( void)
        : m_Action( NULL)
    {}

    ~FsaSupState( void)
    {
        if ( m_Action)
            delete m_Action;
    }

    int32_t     Compare( const FsaSupState &x2) const 
    {
        if ( m_SubStates.size() != x2.m_SubStates.size())
            return m_SubStates.size() < x2.m_SubStates.size() ? 1 : -1;
        const FsaId       *arr1 = &m_SubStates[ 0];
        const FsaId       *arr2 = &x2.m_SubStates[ 0];
        for ( uint32_t i = 0; i < m_SubStates.size(); ++i)
            if ( arr1[ i] != arr2[ i])
                return arr1[ i] < arr2[ i] ? 1 : -1;
        if ( !m_Action != !x2.m_Action)
            return !m_Action > !x2.m_Action ? -1 : 1;
        if ( m_Action == x2.m_Action) 
            return 0;         
        return m_Action->Compare( *x2.m_Action);
    } 

    Cv_CArr< FsaId>             SubStates( void) { return m_SubStates.size() ? Cv_CArr< FsaId>( &m_SubStates[ 0], uint32_t( m_SubStates.size())) : Cv_CArr< FsaId>(); }
 
    std::set< uint32_t>         RuleIds( FsaElemRepos *fsaElemRepos) const
    {  
        std::set< uint32_t>    ruleIds; 
        for ( uint32_t i = 0; i < m_SubStates.size(); ++i)
            ruleIds.insert( fsaElemRepos->RuleIdFromState( m_SubStates[ 0].GetId()));
        return ruleIds;
    }

    void                        PushAction( const Cv_CArr< uint64_t>  &tokens)
    { 
        if ( !tokens.Size())
            return;
        if ( !m_Action)
            m_Action = new Action();
         
        m_Action->Push( tokens);
 
        return;
    }
    
    Action                      *DetachAction( void) 
    { 
        Action  *act = m_Action; 
        m_Action = NULL;
        return act;
    }
    
    Sg_CharDistrib              RefineCharDistrib( FsaRepos *elemRepos);
    FsaDfaState                 *DoConstructTransisition( FsaDfaCnstr *dfaCnstr);
    bool                        WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm);
}; 


//_____________________________________________________________________________________________________________________________ 

struct FsaDfaState  : public FsaState
{ 
    std::vector< FsaId>     m_Dests; 
    Action                  *m_Action;
    
    FsaDfaState( void)
        : m_Action( NULL)
    {}

    ~FsaDfaState( void)
    {
        if ( m_Action)
            delete m_Action;
    }

    Cv_CArr< FsaId>         Dests( void) { return m_Dests.size() ? Cv_CArr< FsaId>( &m_Dests[ 0], uint32_t( m_Dests.size())) : Cv_CArr< FsaId>(); } 
 
    
    bool                    WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm);
    
}; 


//_____________________________________________________________________________________________________________________________ 

struct  FsaElem   : public FsaState
{      
    Action                      *m_Action;
    std::vector< FiltId>        m_ChSets;
    std::vector< FsaId>         m_Dests;

    FsaElem( void)
        : m_Action( NULL)
    {}

    ~FsaElem( void)
    {
        if ( m_Action)
            delete m_Action;
    }

    void            AddEdge( FiltId chSet, FsaId  dest) 
    {
        m_ChSets.push_back( chSet);
        m_Dests.push_back( dest);
    } 

    Cv_CArr< uint64_t>          Tokens( void) { return m_Action ?  Cv_CArr< uint64_t>( &m_Action->m_Values[ 0], uint32_t( m_Action->m_Values.size())) : Cv_CArr< uint64_t>(); } 
    Cv_CArr< FsaId>             Dests( void) { return m_Dests.size() ? Cv_CArr< FsaId>( &m_Dests[ 0], uint32_t( m_Dests.size())) : Cv_CArr< FsaId>(); }  
    Cv_CArr< FiltId>            Filters( void) { return m_ChSets.size() ? Cv_CArr< FiltId>( &m_ChSets[ 0], uint32_t( m_ChSets.size())) : Cv_CArr< FiltId>(); } 
 

    bool                        WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm);
};

//_____________________________________________________________________________________________________________________________ 

struct FsaClip  : public FsaCrate::Var
{
    typedef FsaCrate::Var       FsaVar;
    typedef FsaRepos::Id        FsaId;
    typedef FilterCrate::Var    FiltVar;
    typedef FilterRepos::Id     FiltId;
    
    FsaClip( const FsaVar &v)
        : FsaVar( v)
    {} 

    Cv_CArr< uint64_t>          Tokens( void) { return SELF( [this]( auto k) { return k->Tokens(); }); }

    Cv_CArr< FsaId>             Dests( void) { return SELF( [this]( auto k) { return k->Dests(); }); }
    Cv_CArr< FiltId>            Filters( void) { return SELF( [this]( auto k) { return k->Filters(); }); } 
    Cv_CArr< FsaId>             SubStates( void) { return SELF( [this]( auto k) { return k->SubStates(); }); }
};

//_____________________________________________________________________________________________________________________________ 

struct  FsaDfaStateMap : public Cv_ReposEntry, public Cv_Shared
{   
    typedef std::map< FsaSupState*, FsaDfaState *, Cv_TPtrLess< void> >     SupDfaMap;

    FsaDfaStateMapCltn      *m_Cltn; 
    std::set< uint32_t>     m_Ruleset;
    SupDfaMap               m_SupDfaMap;
    
    FsaDfaStateMap(  const std::set< uint32_t> &ruleset)
        : m_Cltn( NULL), m_Ruleset( ruleset)
    {}

    ~FsaDfaStateMap( void);
  
    int32_t     Compare( const FsaDfaStateMap &dsMap) const
    {
        if ( m_Ruleset.size() != dsMap.m_Ruleset.size())
            return m_Ruleset.size() > dsMap.m_Ruleset.size() ? 1 : -1;
        for ( auto it1 = m_Ruleset.begin(), it2 = dsMap.m_Ruleset.begin(); it1 != m_Ruleset.end(); ++it1, ++it2)
            if ( *it1 != *it2)
                return *it1 < *it2  ? 1 : -1;
        return 0;
    }   
    
    void        Insert(  FsaSupState *supState, FsaDfaState *dfaState)
    {
        m_SupDfaMap.insert( std::pair( supState, dfaState));
    }
    
    FsaDfaState *Find( FsaSupState *supState)
    {
        auto            it = m_SupDfaMap.find( supState);
        if ( it != m_SupDfaMap.end())
           return it->second;
        return NULL;
    }
};

//_____________________________________________________________________________________________________________________________ 

struct  FsaDfaStateMapCltn
{
    FsaDfaCnstr                                         *m_DfaCnstr;
    std::set< FsaDfaStateMap *, Cv_TPtrLess< void> >    m_Maps;
    
    FsaDfaStateMapCltn( FsaDfaCnstr *cnstr)
        : m_DfaCnstr( cnstr)
    {}
    void    Erase( FsaDfaStateMap  *dsMap)
    {
        m_Maps.erase( dsMap);
    }
    
    Cv_Slot< FsaDfaStateMap>    Locate( FsaElemRepos *elemRepos, FsaSupState *supState)
    {
        FsaDfaStateMap  *dfaStatemap = new FsaDfaStateMap( supState->RuleIds( elemRepos));
        auto            it =  m_Maps.lower_bound( dfaStatemap);
        if (( it == m_Maps.end())  || m_Maps.key_comp()( dfaStatemap, *it))
        {   
            m_Maps.insert( it, dfaStatemap);
            dfaStatemap->m_Cltn = this;         
            return dfaStatemap;
        }
        delete dfaStatemap;
        return *it;           
    }
    
    
};
 
//_____________________________________________________________________________________________________________________________ 

struct  FsaDfaCnstr 
{ 

    typedef FsaRepos::Id                FsaId; 
     
    FsaElemRepos                        *m_ElemRepos; 
    FsaRepos                            *m_DfaRepos;               
    std::vector< FsaSupState *>         m_FsaStk;
    FsaDfaStateMapCltn                  m_SupDfaCltn;
    
    FsaDfaCnstr( FsaElemRepos *elemRepos, FsaRepos *dfaRepos)
        : m_ElemRepos( elemRepos), m_DfaRepos( dfaRepos), m_SupDfaCltn( this)
    {}
    
    void    SubsetConstruction( void);
};

//_____________________________________________________________________________________________________________________________ 

};