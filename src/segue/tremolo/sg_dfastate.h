//  sg_dfastate.h _______________________________________________________________________________________________________________
#pragma once

#include    "segue/tremolo/sg_fsastate.h" 

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{ 
//_____________________________________________________________________________________________________________________________ 

struct  FsaDfaRepos  : public FsaRepos
{    
    FsaId               m_RootId;
    FsaElemRepos        *m_ElemRepos;
    DistribRepos        m_DistribRepos;  

    FsaDfaRepos( void)
        : m_ElemRepos( NULL)
    {}
 
    bool                WriteDot( Cv_DotStream &strm);
    bool                DumpDot( const char *path);
    
    bool                DumpStats( std::ostream &ostr)
    {
        FsaRepos::DumpStats( ostr);
        m_DistribRepos.DumpStats( ostr);
        return true;
    }

    struct Cask : public Cv_MemberCask< FsaRepos, FsaId, DistribRepos>
    {  
        typedef Cv_MemberCask< FsaRepos, FsaId, DistribRepos>   BaseCask; 
        typedef typename BaseCask::ContentType                  BaseContent;

        struct  ContentType : public BaseContent
        {  
            ContentType(  const BaseContent &t2)
                : BaseContent( t2)
            {}

            auto        GetM( void) { return ((BaseCask::BaseContent *) this)->m_Value; }
        };

        static uint32_t         ContentSize( const FsaDfaRepos &obj) { return  Cv_Cask< FsaRepos>::ContentSize( obj) 
                                        +Cv_Cask< FsaId>::ContentSize( obj.m_RootId) +Cv_Cask< DistribRepos>::ContentSize( obj.m_DistribRepos); }

        static ContentType      Encase( Cv_Spritz *spritz, const FsaDfaRepos &obj)
        { 
            spritz->EnsureSize( ContentSize( obj)); 
            return BaseCask::Encase( spritz, obj, obj.m_RootId, obj.m_DistribRepos);
        }
 
    }; 

    struct Blossom  
    {
        typedef typename Cask::ContentType    ContentType; 
        typedef typename Cask::BaseContent    SubContent; 

        ContentType                         *m_Root;   
        Blossom( void *arr)
            : m_Root( ( ContentType *) arr)
        {} 
        
        auto    States( void) { return FsaRepos::Blossom( ( uint8_t *) &m_Root->m_Value); }
        auto    RootId( void) { return m_Root->Base()->m_Value; }
        auto    Distribs( void) { return DistribRepos::Blossom(  &m_Root->Base()->Base()->m_Value); }  
    };
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

    Cv_CArr< FsaId>         SubStates( void) { return m_SubStates.size() ? Cv_CArr< FsaId>( &m_SubStates[ 0], uint32_t( m_SubStates.size())) : Cv_CArr< FsaId>(); }

    std::set< uint32_t>     RuleIds( FsaElemRepos *fsaElemRepos) const
    {  
        std::set< uint32_t>    ruleIds; 
        for ( uint32_t i = 0; i < m_SubStates.size(); ++i)
            ruleIds.insert( fsaElemRepos->RuleIdFromState( m_SubStates[ i].GetId()));
        return ruleIds;
    }

    void                    PushAction( const Cv_CArr< uint64_t>  &tokens)
    { 
        if ( !tokens.Size())
            return;
        if ( !m_Action)
            m_Action = new Action(); 
        m_Action->Push( tokens);
        return;
    }

    void                    Freeze( void)
    {
        std::sort( m_SubStates.begin(), m_SubStates.end());
        auto    it = std::unique( m_SubStates.begin(), m_SubStates.end());
        m_SubStates.resize( std::distance( m_SubStates.begin(), it));
        if ( !m_Action)
            return;
        std::sort( m_Action->m_Values.begin(), m_Action->m_Values.end()); 
        auto    itAct = std::unique( m_Action->m_Values.begin(), m_Action->m_Values.end());
        m_Action->m_Values.resize( std::distance( m_Action->m_Values.begin(), itAct));
        return;
    }

    Action                  *DetachAction( void) 
    { 
        Action  *act = m_Action; 
        m_Action = NULL;
        return act;
    } 

    void                    DoConstructTransisition( FsaDfaCnstr *dfaCnstr);
    bool                    WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm); 

    struct ElemIt
    {
        FsaElemRepos        *m_ElemRepos;
        Cv_CArr< FsaId>     m_SubStates;
        uint32_t            m_StateCursor;

        ElemIt( FsaElemRepos *elemRepos,  FsaSupState *supState)
            :  m_ElemRepos( elemRepos), m_SubStates( supState->SubStates()), m_StateCursor( 0)
        {}
        
        void                Reset( void) { m_StateCursor = 0; }

        bool                IsCurValid( void) { return m_StateCursor < m_SubStates.Size(); }
        FsaElemRepos::Var   Curr( void) { return m_ElemRepos->ToVar( m_SubStates[ m_StateCursor]); }
        bool                Next( void) 
        { 
            if ( ++m_StateCursor < m_SubStates.Size())
                return true;
            return false; 
        } 
    };

    struct DescendIt : public ElemIt
    { 
        Cv_CArr< FiltId>                m_Filters;
        Cv_CArr< FsaId>                 m_DestStateIds;
        uint32_t                        m_FilterCursor; 
        FsaDfaRepos                     *m_DfaRepos;
        std::vector< FsaSupState *>     m_SubSupStates; 
 
        DescendIt( FsaElemRepos *elemRepos, FsaDfaRepos *dfaRepos, FsaSupState *supState)
            : ElemIt( elemRepos, supState), m_FilterCursor( 0), m_DfaRepos( dfaRepos)
        {
            if ( ElemIt::IsCurValid())
                FetchFilterElems();
        }
    
        void                Reset( void) 
        { 
            ElemIt::Reset(); 
            m_FilterCursor = 0; 
            if ( ElemIt::IsCurValid()) 
                FetchFilterElems(); 
        }
    
        void                DoSetup( uint32_t szDescend)
        { 
            Reset();
            for ( uint32_t k = 0; k < szDescend; ++k)
            {
                FsaSupState     *subSupState = new FsaSupState();
                m_SubSupStates.push_back( subSupState);
            }
        } 

        bool                IsCurValid( void) { return ElemIt::IsCurValid() && ( m_FilterCursor < m_Filters.Size()); }
        FilterRepos::Var    CurrFilt( void) { return m_ElemRepos->m_FilterRepos.ToVar( m_Filters[ m_FilterCursor]); }
        FsaCrate::Var       CurrDest( void) { return m_ElemRepos->ToVar( m_DestStateIds[ m_FilterCursor]); }
        
        bool                Next( void) 
        { 
            if ( ++m_FilterCursor < m_Filters.Size())
                return true;
            m_FilterCursor = 0;
            if ( ElemIt::Next())
            {
                FetchFilterElems(); 
                return true;
            }              
            return false; 
        } 
        
        void        FetchFilterElems( void);

    template < uint32_t BitSz> 
        void        Classify( const CharDistrib< BitSz> &distrib)
        {   
            while ( IsCurValid())
            { 
                FilterCrate::Var            chSet = CurrFilt();
                Sg_Bitset< BitSz>           *bitset = static_cast< ChSetFilter< BitSz> *>( chSet.GetEntry());
                Cv_Array< uint8_t, BitSz>   images = distrib.CCLImages( *bitset); 
                FsaCrate::Var               dest = CurrDest();
                for ( uint32_t k = 0; k < images.SzFill(); ++k)
                { 
                    FsaSupState     *subSupState = m_SubSupStates[ images[ k]];
                    subSupState->m_SubStates.push_back( FsaDfaRepos::ToId( dest));  
                    subSupState->PushAction( dest( []( auto elem) { return elem->Tokens(); })); 
                }
                Next();
            }    
            return;
        } 

        void        Dump( std::ostream &strm);
    };
}; 

//_____________________________________________________________________________________________________________________________ 

struct FsaDfaState  : public FsaState
{  
private:
    DistribRepos::Discr     m_Discr;
    uint16_t                m_TokSz;         
    
    FsaDfaState( const DistribRepos::Discr &discr, uint8_t tokSz) 
        : m_Discr( discr), m_TokSz( tokSz)
    {}

public:
    struct Cask : public Cv_SerializeUtils 
    {      
        typedef FsaDfaState        Type;
        typedef FsaDfaState        ContentType;  

        static uint32_t             Spread( ContentType *obj) 
        {   
            return sizeof( *obj) + obj->DestSz() * sizeof( FsaId)  + obj->m_TokSz * sizeof( uint64_t); 
        }

        static const ContentType    &Encase( Cv_Spritz *spritz, const FsaDfaState &obj) 
        {  
            FsaDfaState             &dfaState = const_cast< FsaDfaState &>( obj);
            bool                    res = spritz->Write( &dfaState, sizeof( dfaState)); 
            Cv_CArr< FsaId>         dests = dfaState.Dests();  
            res = spritz->Write( &dests[ 0], sizeof( FsaId) *  dests.Size()); 
            Cv_CArr< uint64_t>      toks =  dfaState.Tokens();   
            res = spritz->Write( &toks[ 0], sizeof( uint64_t) *  toks.Size()); 
            return obj; 
        }  

    template < typename Spritz>
        static void   SaveContent( Spritz *spritz, const FsaDfaState &obj) 
        { 
            return;
        }
    };

    typedef FsaDfaRepos     Repos;

    ~FsaDfaState( void)
    {} 
    
    void            Delete( FsaDfaRepos *repos)
    {
        this->FsaDfaState::~FsaDfaState(); 
        delete [] ( uint8_t *) this;
    }
     
    uint8_t                 *PastPtr( void) { return reinterpret_cast< uint8_t *>( this) +sizeof( FsaDfaState); }
    static FsaDfaState      *Construct( const DistribRepos::Discr &discr, Action *action)
    {
        uint32_t            sz = discr.SzDescend();
        uint8_t             szTok = action  ? uint8_t( action->m_Values.size()) : 0;
        auto                memSz = sizeof( FsaDfaState) + sz * sizeof( FsaId) +  szTok * sizeof( uint64_t);
        FsaDfaState         *dfaState = new ( new uint8_t[ memSz]) FsaDfaState( discr, szTok); 
        uint64_t            *toks = dfaState->Tokens().Ptr();
        for ( uint32_t i = 0; i < szTok; ++i)
            toks[ i] = action->m_Values[ i];
        return dfaState;
    }
    uint32_t                DestSz( void) const { return m_Discr.SzDescend(); }
    
    DistribRepos::Id        DistribId( void) const { return m_Discr.m_DId; }

    Cv_CArr< FsaId>         Dests( void) { return DestSz() ? Cv_CArr< FsaId>( ( FsaId *) PastPtr(), uint32_t( DestSz())) : Cv_CArr< FsaId>(); } 
    void                    SetDest( uint8_t k, FsaId fsaId) {   Dests()[ k] = fsaId; }

    Cv_CArr< uint64_t>      Tokens( void) { return m_TokSz ? Cv_CArr< uint64_t>( ( uint64_t *) ( PastPtr() + DestSz() * sizeof( FsaId)), uint32_t( m_TokSz)) : Cv_CArr< uint64_t>(); } 

    bool                    CleanupDestIds( FsaRepos *dfaRepos);

    bool                    WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm);
    bool                    DumpDot( Cv_DotStream &strm); 
}; 

//_____________________________________________________________________________________________________________________________ 

struct FsaClip  : public FsaCrate::Var
{
    typedef FsaCrate::Var       FsaVar;
    typedef FsaRepos::Id        FsaId;
    typedef FilterCrate::Var    FiltVar;
    typedef FilterRepos::Id     FiltId;
    
    FsaClip( void)
    {}

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
    typedef std::map< FsaSupState*, uint32_t, Cv_TPtrLess< void> >     SupDfaMap;

    FsaDfaStateMapCltn      *m_Cltn; 
    std::set< uint32_t>     m_Ruleset;
    SupDfaMap               m_SupDfaMap;

    FsaDfaStateMap(  const std::set< uint32_t> &ruleset)
        : m_Cltn( NULL), m_Ruleset( ruleset)
    {}

    ~FsaDfaStateMap( void);

    int32_t                 Compare( const FsaDfaStateMap &dsMap) const
    {
        if ( m_Ruleset.size() != dsMap.m_Ruleset.size())
            return m_Ruleset.size() > dsMap.m_Ruleset.size() ? 1 : -1;
        for ( auto it1 = m_Ruleset.begin(), it2 = dsMap.m_Ruleset.begin(); it1 != m_Ruleset.end(); ++it1, ++it2)
            if ( *it1 != *it2)
                return *it1 < *it2  ? 1 : -1;
        return 0;
    }   

    void                    Insert(  FsaSupState *supState, uint32_t stateId)
    {
        m_SupDfaMap.insert( std::make_pair( supState, stateId));
    }

    uint32_t    Find( FsaSupState *supState)
    {
        auto            it = m_SupDfaMap.find( supState);
        if ( it != m_SupDfaMap.end())
            return it->second;
        return CV_UINT32_MAX;
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
    FsaDfaRepos                         *m_DfaRepos;               
    std::vector< FsaSupState *>         m_FsaStk;
    FsaDfaStateMapCltn                  m_SupDfaCltn;

    FsaDfaCnstr( FsaElemRepos *elemRepos, FsaDfaRepos *dfaRepos)
        : m_ElemRepos( elemRepos), m_DfaRepos( dfaRepos), m_SupDfaCltn( this)
    {
        dfaRepos->m_ElemRepos = elemRepos;
        dfaRepos->m_DistribRepos.m_Base = elemRepos->m_FilterRepos.m_Base;
    }

    void        SubsetConstruction( void);
    bool        DumpDot( const char *path);
};


//_____________________________________________________________________________________________________________________________ 

};