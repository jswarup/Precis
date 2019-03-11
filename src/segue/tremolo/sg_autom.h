//  sg_autom.h ___________________________________________________________________________________________________________________
#pragma once

#include    "cove/barn/cv_cexpr.h"
#include 	"cove/barn/cv_aid.h"
#include 	"cove/barn/cv_ptrslot.h"
#include    "segue/timbre/sg_timbreparser.h"
#include    "segue/grammar/sg_rexpgrammar.h"

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{
//_____________________________________________________________________________________________________________________________ 

struct AutomCnstr;
typedef Cv_Slot< AutomCnstr> AutomSlot;
//_____________________________________________________________________________________________________________________________ 

struct  AutomCnstr   : public Cv_ReposEntry, public Cv_Shared
{ 
    std::vector< Sg_ChSet>      m_ChSets;
    std::vector< AutomSlot>     m_Dests;
    std::set< AutomSlot>        m_EpsDestIds; 
    std::set< AutomSlot>        m_EpsFromSources;

public:
    AutomCnstr( void)  
    {} 
    
    virtual ~AutomCnstr( void)  
    {} 

    const char		*GetName( void) const { return "Spur"; }

    void        AddEdge( const Sg_ChSet &chSet, const AutomSlot &dest) 
    {
        m_ChSets.push_back( chSet);
        m_Dests.push_back( dest);
    }

    void        AddEpsDest( const AutomSlot &s) 
    { 
        if ( this == s.Ptr())
            return; 
        m_EpsDestIds.insert( s);    
        s->AddEpsSource( this); 
        return;
    }

    void    AddEpsSource( const AutomSlot &s) 
    { 
        if ( this == s.Ptr())
            return; 
        m_EpsFromSources.insert( s);  
        return;
    }

    bool    WriteDot( Cv_DotStream &strm)  
    {
        strm << 'R' << GetId() << " [ shape=ellipse color=cyan label= <<FONT> N" << GetId() << "<BR />" ; 
        strm << " </FONT>>];\n "; 

        for ( uint32_t k = 0; k < m_Dests.size(); ++k)
        {
            const AutomSlot     &regex = m_Dests[ k];
            strm << 'R' << GetId() << " -> " << 'R' << regex->GetId() << " [ arrowhead=normal color=black label=<<FONT> ";  
            strm << Cv_Aid::XmlEncode(  m_ChSets[ k].ToString());
            strm << "</FONT>>] ; \n" ;  
        } 

        for ( auto it = m_EpsDestIds.begin(); it !=  m_EpsDestIds.end(); ++it) 
            strm << 'R' << GetId() << " -> " << 'R' << (*it)->GetId() << " [ arrowhead=vee color=blue] ; \n"; 

        for ( auto it = m_EpsFromSources.begin(); it !=  m_EpsFromSources.end(); ++it)  
            strm << 'R' << (*it)->GetId() << " -> " << 'R' << GetId() << " [ arrowhead=tee color=green] ; \n"; 
        return true;
    }
};

//_____________________________________________________________________________________________________________________________  
 
struct AutomCnstrRepos  
{
    RExpRepos				    *m_RexpRepos;
    std::vector< AutomSlot>     m_Elems;
    
    AutomCnstrRepos(  RExpRepos *rexpRepos)
        : m_RexpRepos( rexpRepos)
    {}
    
template<  class Object>
    uint32_t    Store( Object *x)
    {
        
        return ind;
    } 

    AutomSlot   Construct( void)
    {
        AutomSlot   x = new AutomCnstr();
        uint32_t    ind = m_Elems.size();
        x->SetId( ind);
        m_Elems.push_back( x); 
        return x;
    }

    void    Proliferate( SynElem *elm, const AutomSlot &start, const AutomSlot &end)
    {
        return;   
    }

    void    Proliferate( CSetSynElem *csetElm, const AutomSlot &start, const AutomSlot &end)
    {
        start->AddEdge( csetElm->m_Filt, end);
    }

    void    Proliferate( SeqSynElem *seqElm, const AutomSlot &start, const AutomSlot &end)
    {
          
        auto    right = end; 
        for ( uint32_t i = uint32_t( seqElm->m_SeqList.size()); i > 1; --i)
        {
            AutomSlot           state =  Construct(); 
            RExpCrate::Var      elemVar = m_RexpRepos->ToVar( seqElm->m_SeqList[ i -1]); 

            elemVar( [ this, state, right](  auto k) {
                Proliferate( k, state, right);
            });
            right = state;
        }
        RExpCrate::Var      elemVar = m_RexpRepos->ToVar( seqElm->m_SeqList[0]); 

        elemVar( [ this, start, right](  auto k) {
            Proliferate( k, start, right);
        });
        return;   
    }

    void    Proliferate( AltSynElem *altElm, const AutomSlot &start, const AutomSlot &end)
    {
        for ( uint32_t i = 0; i < altElm->m_AltList.size(); ++i)
        {
            AutomSlot       state =  Construct();
            start->AddEpsDest( state);

            RExpCrate::Var      elemVar = m_RexpRepos->ToVar( altElm->m_AltList[ i]);

            elemVar( [ this, state, end](  auto k) {
                 Proliferate( k, state, end);
            });
        }
        return;   
    }
        
    void    Proliferate( RepeatSynElem *repElm, const AutomSlot &start, const AutomSlot &end)
    {
        uint32_t            nSeg  = ( repElm->m_Max == 0) ? ( repElm->m_Min +1) : repElm->m_Max; 
        AutomSlot           fState = Construct();
        start->AddEpsDest( fState);
        
        AutomSlot                   sState;
        AutomSlot                   loopbackState;
        std::vector< AutomSlot>     optSpinList;
        RExpCrate::Var              elemVar = m_RexpRepos->ToVar( repElm->m_Elem);
        for ( uint32_t i = 0; i < nSeg; ++i)
        { 
            sState = Construct();
            elemVar( [ this, fState, sState](  auto k) {
                Proliferate( k, fState, sState);
            });
            if (( repElm->m_Max && ( i >= repElm->m_Min)) || ( !repElm->m_Max && ( i == repElm->m_Min)))
                optSpinList.push_back( fState);
            if ( !repElm->m_Max && ( i == repElm->m_Min))
                loopbackState = fState;
            fState = sState;
        }
        for ( auto it = optSpinList.begin(); it != optSpinList.end(); ++it)
            (*it)->AddEpsDest( end);

        if ( repElm->m_Max) 
            sState->AddEpsDest( end); 
        else
            sState->AddEpsDest( loopbackState);
        return;   
    }

    void    Process( void)
    {
        AutomSlot       start =  Construct();
        AutomSlot       end =  Construct();
        RExpCrate::Var  docVar = m_RexpRepos->ToVar( m_RexpRepos->m_RootId);
        docVar( [ this, start, end](  auto k) {
            Proliferate( k, start, end);
        });
        return;
    }
};

//_____________________________________________________________________________________________________________________________  
};

//_____________________________________________________________________________________________________________________________  

