//  sg_dfastate.h _______________________________________________________________________________________________________________
#pragma once

#include    "segue/tremolo/sg_fsastate.h" 

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{


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
            ruleIds.insert( fsaElemRepos->RuleIdFromState( m_SubStates[ i].GetId()));
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

    void                        Freeze( void)
    {
        std::sort( m_SubStates.begin(), m_SubStates.end());
        if ( m_Action)
            std::sort( m_Action->m_Values.begin(), m_Action->m_Values.end()); 
        return;
    }

    Action                      *DetachAction( void) 
    { 
        Action  *act = m_Action; 
        m_Action = NULL;
        return act;
    }

    Sg_Partition               RefineCharDistrib( FsaRepos *elemRepos);
    FsaDfaState                 *DoConstructTransisition( FsaDfaCnstr *dfaCnstr);
    bool                        WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm);
}; 

//_____________________________________________________________________________________________________________________________ 

struct FsaDfaState  : public FsaState
{  
private:
    uint16_t                 m_DestSz;
    uint16_t                 m_TokSz; 

    FsaDfaState( uint8_t dSz, uint8_t tokSz) 
        : m_DestSz( dSz), m_TokSz( tokSz)
    {}

public:
    ~FsaDfaState( void)
    {} 

    uint8_t                 *PastPtr( void) { return reinterpret_cast< uint8_t *>( this) +sizeof( FsaDfaState); }
    static FsaDfaState      *Construct( uint8_t sz, Action *action)
    {
        uint8_t         szTok = action  ? uint8_t( action->m_Values.size()) : 0;
        auto            memSz = sizeof( FsaDfaState) + sz * sizeof( FsaId) +  szTok * sizeof( uint64_t);
        FsaDfaState     *dfaState = new (new uint8_t[ memSz]) FsaDfaState( sz, szTok); 
        uint64_t        *toks = dfaState->Tokens().Ptr();
        for ( uint32_t i = 0; i < szTok; ++i)
            toks[ i] = action->m_Values[ i];
        return dfaState;
    }

    Cv_CArr< FsaId>         Dests( void) { return m_DestSz ? Cv_CArr< FsaId>( ( FsaId *) PastPtr(), uint32_t( m_DestSz)) : Cv_CArr< FsaId>(); } 
    void                    SetDest( uint8_t k, FsaId fsaId) {   Dests()[ k] = fsaId; }

    Cv_CArr< uint64_t>      Tokens( void) { return m_TokSz ? Cv_CArr< uint64_t>( ( uint64_t *) ( PastPtr() + m_DestSz * sizeof( FsaId)), uint32_t( m_TokSz)) : Cv_CArr< uint64_t>(); } 

    bool                    WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm);

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