//  sg_fsaelemcnstr.h ___________________________________________________________________________________________________________________
#pragma once

#include    "segue/colophon/sg_fsastate.h"
#include    "segue/colophon/sg_dfastate.h"
#include    "segue/epigraph/sg_parser.h"
#include    "segue/grammar/sg_rexpgrammar.h"
#include    "segue/epigraph/sg_partitiondb.h" 

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{
//_____________________________________________________________________________________________________________________________ 

struct      FsaElemCnstr;
struct      FsaElemReposCnstr;

typedef Cv_Slot< FsaElemCnstr>    ElemSlot;

//_____________________________________________________________________________________________________________________________ 

struct  FsaElemCnstr   : public Cv_ReposEntry, public Cv_Shared
{ 
    FsaElemReposCnstr           *m_Repos;
    FsaElem                     *m_State;
    std::set< FsaElemCnstr *>   m_EpsDests; 
    std::set< uint32_t>         m_EpsSourceIds;

public:
    FsaElemCnstr( FsaElemReposCnstr *repos)  
        : m_Repos( repos)
    {}  
 
    ~FsaElemCnstr( void); 
    
    const char		*GetName( void) const { return "Spur"; }

    void            AddEdge( const Sg_ChSet &chSet, const ElemSlot &dest);

    void            AddEpsDest( const ElemSlot &s) 
    { 
        if ( this == s.Ptr())
            return; 
        m_EpsDests.insert( s);    
        s->AddEpsSource( GetId()); 
        return;
    }

    void            AddEpsSource( uint32_t sId) 
    { 
        if ( GetId() == sId)
            return; 
        m_EpsSourceIds.insert( sId);  
        return;
    }

    void            FinalizeEpsLinks( void);
    
    bool            WriteDot( FsaElem::Id id, Cv_DotStream &strm) ;
};

//_____________________________________________________________________________________________________________________________  
 
struct FsaElemReposCnstr 
{
    RExpRepos				        *m_RexpRepos; 
    FsaElemRepos                    *m_ElemRepos;
    std::vector< FsaElemCnstr *>    m_Cnstrs;

    FsaElemReposCnstr(  RExpRepos *rexpRepos, FsaElemRepos *automRepos)
        : m_RexpRepos( rexpRepos), m_ElemRepos( automRepos)
    { 
        m_Cnstrs.push_back( NULL);
        m_ElemRepos->m_RuleIdSzList.resize( m_RexpRepos->m_RuleSz, 0);
        m_ElemRepos->m_FilterRepos.m_Base = rexpRepos->m_Base;
    } 

    ElemSlot   ConstructCnstr( void)
    {
        ElemSlot   x = new FsaElemCnstr( this);  
        x->SetId( m_ElemRepos->Size()); 
        x->m_State = m_ElemRepos->Construct< FsaElem>();  
        m_Cnstrs.push_back( x);
        return x;
    }

    void    Proliferate( SynElem *elm, const ElemSlot &start, const ElemSlot &end)
    {
        return;   
    }
    
    void    Proliferate( ActionSynElem *elm, const ElemSlot &start, const ElemSlot &end)
    {

        ElemSlot       actor =  ConstructCnstr();
        actor->AddEpsDest( end);

        RExpCrate::Var      elemVar = m_RexpRepos->ToVar( elm->m_Elem); 

        elemVar( [ this, &start, &actor](  auto k) {
                Proliferate( k, start, actor);
            });
        if ( !actor->m_State->m_Action)
            actor->m_State->m_Action = new Action();
        actor->m_State->m_Action->Push( elm->m_Token);
        return;   
    }

    void    Proliferate( CSetSynElem *csetElm, const ElemSlot &start, const ElemSlot &end)
    {
        start->AddEdge( csetElm->m_Filt, end);
        return;   
    }

    void    Proliferate( SeqSynElem *seqElm, const ElemSlot &start, const ElemSlot &end)
    { 
        ElemSlot    right = end; 
        for ( uint32_t i = uint32_t( seqElm->m_SeqList.size()); i > 1; --i)
        {
            ElemSlot           state =  ConstructCnstr(); 
            RExpCrate::Var      elemVar = m_RexpRepos->ToVar( seqElm->m_SeqList[ i -1]); 

            elemVar( [ this, &state, &right](  auto k) {
                Proliferate( k, state, right);
            });
            right = state;
        }
        RExpCrate::Var      elemVar = m_RexpRepos->ToVar( seqElm->m_SeqList[0]); 

        elemVar( [ this, &start, &right](  auto k) {
            Proliferate( k, start, right);
        });
        return;   
    }

    void    Proliferate( RExpEntrySeqElem *seqElm, const ElemSlot &start, const ElemSlot &end)
    { 
        Proliferate( static_cast< SeqSynElem *>( seqElm), start, end);
        m_ElemRepos->m_RuleIdSzList[ seqElm->m_RuleIndex] = m_ElemRepos->Size(); 
    }

    void    Proliferate( AltSynElem *altElm, const ElemSlot &start, const ElemSlot &end)
    {
        for ( uint32_t i = 0; i < altElm->m_AltList.size(); ++i)
        {
            ElemSlot       state =  ConstructCnstr();
            start->AddEpsDest( state);

            RExpCrate::Var      elemVar = m_RexpRepos->ToVar( altElm->m_AltList[ i]);

            elemVar( [ this, &state, &end](  auto k) {
                 Proliferate( k, state, end);
            });
        }
        return;   
    }
        
    void    Proliferate( RepeatSynElem *repElm, const ElemSlot &start, const ElemSlot &end)
    {
        uint32_t            nSeg  = ( repElm->m_Max == 0) ? ( repElm->m_Min +1) : repElm->m_Max; 
        
        ElemSlot           loopbackState;
        ElemSlot           sState;
        {
            std::vector< ElemSlot>     optSpinList;
            {
                ElemSlot           fState = ConstructCnstr();
                start->AddEpsDest( fState);
                RExpCrate::Var              elemVar = m_RexpRepos->ToVar( repElm->m_Elem);
                for ( uint32_t i = 0; i < nSeg; ++i)
                { 
                    sState = ConstructCnstr();
                    elemVar( [ this, fState, sState](  auto k) {
                        Proliferate( k, fState, sState);
                    });
                    if (( repElm->m_Max && ( i >= repElm->m_Min)) || ( !repElm->m_Max && ( i == repElm->m_Min)))
                        optSpinList.push_back( fState);
                    if ( !repElm->m_Max && ( i == repElm->m_Min))
                        loopbackState = fState;
                    fState = sState;
                } 
            }
            for ( auto it = optSpinList.begin(); it != optSpinList.end(); ++it)
                (*it)->AddEpsDest( end);
        }

        if ( repElm->m_Max) 
            sState->AddEpsDest( end); 
        else
            sState->AddEpsDest( loopbackState);
        return;   
    }

    bool    WriteDot( const std::string &str);

    void    Process( void);
};

//_____________________________________________________________________________________________________________________________  
};

//_____________________________________________________________________________________________________________________________  

