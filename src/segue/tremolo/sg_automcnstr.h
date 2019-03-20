//  sg_automcnstr.h ___________________________________________________________________________________________________________________
#pragma once


#include    "segue/tremolo/sg_fsastate.h"
#include    "segue/timbre/sg_timbreparser.h"
#include    "segue/grammar/sg_rexpgrammar.h"

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{
//_____________________________________________________________________________________________________________________________ 

struct      AutomCnstr;
struct      AutomReposCnstr;

typedef Cv_Slot< AutomCnstr>    AutomSlot;

//_____________________________________________________________________________________________________________________________ 

struct  AutomCnstr   : public Cv_ReposEntry, public Cv_Shared
{ 
    AutomReposCnstr                 *m_Repos;
    FsaElem                         *m_State;
    std::set< AutomCnstr *>         m_EpsDests; 
    std::set< uint32_t>             m_EpsSourceIds;

public:
    AutomCnstr( AutomReposCnstr *repos)  
        : m_Repos( repos)
    {}  
 
    ~AutomCnstr( void); 
    
    const char		*GetName( void) const { return "Spur"; }

    void            AddEdge( const Sg_ChSet &chSet, const AutomSlot &dest);

    void            AddEpsDest( const AutomSlot &s) 
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
    
    bool            WriteDot( Cv_DotStream &strm) ;
};

//_____________________________________________________________________________________________________________________________  
 
struct AutomReposCnstr 
{
    FsaRepos                        m_AutomRepos;
    RExpRepos				        *m_RexpRepos; 
    std::vector< AutomCnstr *>      m_Cnstrs;

    AutomReposCnstr(  RExpRepos *rexpRepos)
        : m_RexpRepos( rexpRepos)
    { 
        m_Cnstrs.push_back( NULL);
    } 

    AutomSlot   ConstructCnstr( void)
    {
        AutomSlot   x = new AutomCnstr( this);  
        x->SetId( m_AutomRepos.Size()); 
        x->m_State = m_AutomRepos.Construct< FsaElem>();  
        m_Cnstrs.push_back( x);
        return x;
    }

    void    Proliferate( SynElem *elm, const AutomSlot &start, const AutomSlot &end)
    {
        return;   
    }
    
    void    Proliferate( ActionSynElem *elm, const AutomSlot &start, const AutomSlot &end)
    {
        RExpCrate::Var      elemVar = m_RexpRepos->ToVar( elm->m_Elem); 

        elemVar( [ this, &start, &end](  auto k) {
                Proliferate( k, start, end);
            });
        end->m_State->m_Action = new Action( elm->m_Token);
        return;   
    }

    void    Proliferate( CSetSynElem *csetElm, const AutomSlot &start, const AutomSlot &end)
    {
        start->AddEdge( csetElm->m_Filt, end);
        return;   
    }

    void    Proliferate( SeqSynElem *seqElm, const AutomSlot &start, const AutomSlot &end)
    { 
        AutomSlot    right = end; 
        for ( uint32_t i = uint32_t( seqElm->m_SeqList.size()); i > 1; --i)
        {
            AutomSlot           state =  ConstructCnstr(); 
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

    void    Proliferate( AltSynElem *altElm, const AutomSlot &start, const AutomSlot &end)
    {
        for ( uint32_t i = 0; i < altElm->m_AltList.size(); ++i)
        {
            AutomSlot       state =  ConstructCnstr();
            start->AddEpsDest( state);

            RExpCrate::Var      elemVar = m_RexpRepos->ToVar( altElm->m_AltList[ i]);

            elemVar( [ this, &state, &end](  auto k) {
                 Proliferate( k, state, end);
            });
        }
        return;   
    }
        
    void    Proliferate( RepeatSynElem *repElm, const AutomSlot &start, const AutomSlot &end)
    {
        uint32_t            nSeg  = ( repElm->m_Max == 0) ? ( repElm->m_Min +1) : repElm->m_Max; 
        
        AutomSlot           loopbackState;
        AutomSlot           sState;
        {
            std::vector< AutomSlot>     optSpinList;
            {
                AutomSlot           fState = ConstructCnstr();
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

