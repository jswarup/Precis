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

struct  AutomState   : public Cv_ReposEntry, public Cv_Shared
{     
    std::vector< Sg_ChSet>          m_ChSets;
    std::vector< AutomState *>      m_Dests;

    void        AddEdge( const Sg_ChSet &chSet, AutomState *dest) 
    {
        m_ChSets.push_back( chSet);
        m_Dests.push_back( dest);
    } 


    bool        WriteDot( Cv_DotStream &strm)  
    {
        strm << 'R' << GetId() << " [ shape=ellipse color=cyan label= <<FONT> N" << GetId() << "<BR />" ; 
        strm << RefCount() << " </FONT>>];\n "; 

        for ( uint32_t k = 0; k < m_Dests.size(); ++k)
        {
            AutomState      *regex = m_Dests[ k];
            strm << 'R' << GetId() << " -> " << 'R' << regex->GetId() << " [ arrowhead=normal color=black label=<<FONT> ";  
            strm << Cv_Aid::XmlEncode(  m_ChSets[ k].ToString());
            strm << "</FONT>>] ; \n" ;  
        }
        return true;
    }
};

//_____________________________________________________________________________________________________________________________ 

struct  AutomCnstr   : public Cv_ReposEntry, public Cv_Shared
{ 
    AutomState                      *m_State;
    std::set< AutomCnstr *>         m_EpsDests; 
    std::set< AutomCnstr *>         m_EpsSources;

public:
    AutomCnstr( void)  
    {}  
    
    virtual    ~AutomCnstr( void)  
    {
       
    }
    

    uint32_t		LowerRef( void) 
    {	
        if ( m_RefCount == 2)
            FinalizeEpsLinks(); 
        return --m_RefCount;	
    }
    
    const char		*GetName( void) const { return "Spur"; }

    void        AddEdge( const Sg_ChSet &chSet, const AutomSlot &dest) 
    {
        m_State->AddEdge( chSet, dest->m_State);
    }

    void        AddEpsDest( const AutomSlot &s) 
    { 
        if ( this == s.Ptr())
            return; 
        m_EpsDests.insert( s);    
        s->AddEpsSource( this); 
        return;
    }

    void    AddEpsSource( AutomCnstr *s) 
    { 
        if ( this == s)
            return; 
        m_EpsSources.insert( s);  
        return;
    }

    bool    WriteDot( Cv_DotStream &strm)  
    {
         
        for ( auto it = m_EpsDests.begin(); it !=  m_EpsDests.end(); ++it) 
            strm << 'R' << GetId() << " -> " << 'R' << (*it)->GetId() << " [ arrowhead=vee color=blue] ; \n"; 

        for ( auto it = m_EpsSources.begin(); it !=  m_EpsSources.end(); ++it)  
            strm << 'R' << (*it)->GetId() << " -> " << 'R' << GetId() << " [ arrowhead=tee color=green] ; \n"; 
        return true;
    }
    
     //_____________________________________________________________________________________________________________________________   

    void    FinalizeEpsLinks( void)
    {
        if (  !m_EpsSources.size())        // nothing needs to be done if no one has eps transition to it.
            return;

        // state is frozen: It should meet its obligation to export its OutTransitions to its eps-sourcces.
        for ( auto sIt = m_EpsSources.begin(); sIt != m_EpsSources.end(); ++sIt)
        {
            if ( (*sIt) == this)
                continue;  
            // export all this state transistions to the eps-source  
            for ( uint32_t i = 0; i < m_State->m_Dests.size(); ++i) 
            {
                (*sIt)->m_State->AddEdge( m_State->m_ChSets[ i], m_State->m_Dests[ i]); 
            }
            // export all this state eps-transistions to the eps-source  
            for ( auto dIt = m_EpsDests.begin(); dIt != m_EpsDests.end(); ++dIt)
                (*sIt)->AddEpsDest( *dIt); 

            (*sIt)->m_EpsDests.erase( this);
        }
        for ( auto dIt = m_EpsDests.begin(); dIt != m_EpsDests.end(); ++dIt)
            (*dIt)->m_EpsSources.erase( this);
        m_EpsSources.clear();
        m_EpsDests.clear(); 
        return;
    }
};

//_____________________________________________________________________________________________________________________________  
 
struct AutomRepos 
{
    Cv_Repos< AutomState>           m_AutomRepos;
    RExpRepos				        *m_RexpRepos;
    AutomSlot                       m_Start;   
    AutomSlot                       m_End;   
    std::vector< AutomSlot>         m_Cnstrs;

    AutomRepos(  RExpRepos *rexpRepos)
        : m_RexpRepos( rexpRepos)
    {
        m_Cnstrs.push_back( NULL);
    }
    
template<  class Object>
    uint32_t    Store( Object *x)
    {
        
        return ind;
    } 

    AutomSlot   ConstructCnstr( void)
    {
        AutomSlot   x = new AutomCnstr(); 
        x->m_State = m_AutomRepos.Construct< AutomState>();
        uint32_t    ind = m_Cnstrs.size();
        x->SetId( ind); 
        m_Cnstrs.push_back( x); 
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
        AutomSlot           fState = ConstructCnstr();
        start->AddEpsDest( fState);
        
        AutomSlot                   sState;
        AutomSlot                   loopbackState;
        std::vector< AutomSlot>     optSpinList;
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
        for ( auto it = optSpinList.begin(); it != optSpinList.end(); ++it)
            (*it)->AddEpsDest( end);

        if ( repElm->m_Max) 
            sState->AddEpsDest( end); 
        else
            sState->AddEpsDest( loopbackState);
        return;   
    }

    bool    WriteDot( const std::string &str)
    {
        std::ofstream           rexpOStrm( str);
        Cv_DotStream			rexpDotStrm( &rexpOStrm, true);  

        for ( uint32_t i = 1; i < m_AutomRepos.Size(); ++i)
        {
            AutomState  *si = m_AutomRepos.At( i);
            if (si)
                si->WriteDot( rexpDotStrm); 
        }

        for ( uint32_t i = 1; i < m_Cnstrs.size(); ++i)
        {
            const AutomSlot    &si = m_Cnstrs[ i];
            if (si)
                si->WriteDot( rexpDotStrm); 
        }
        return true;
    }
    void    Process( void)
    {
        m_Start =  ConstructCnstr();
        m_End =  ConstructCnstr();
        RExpCrate::Var  docVar = m_RexpRepos->ToVar( m_RexpRepos->m_RootId);
        docVar( [ this](  auto k) {
            Proliferate( k, m_Start, m_End);
        });

/*        for ( uint32_t i = 1; i < m_Elems.size(); ++i)
        {
            auto    si = m_Elems[ i];
            si->FinalizeEpsLinks();
        }
*/        return;
    }
};

//_____________________________________________________________________________________________________________________________  
};

//_____________________________________________________________________________________________________________________________  

