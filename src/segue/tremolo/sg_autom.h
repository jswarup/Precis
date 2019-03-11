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
  
struct     AutomSpurCnstr; 
struct     AutomCnstr;

typedef Cv_Crate<  AutomSpurCnstr, AutomCnstr>      AutomCnstrCrate;   

//_____________________________________________________________________________________________________________________________ 

struct  AutomCnstr   : public Cv_CrateEntry, public Cv_Shared
{ 
public:
    AutomCnstr( void)  
    {} 
    
    virtual ~AutomCnstr( void)  
    {} 

    void    AddEdge( ...)  {}

    void    AddEpsDest( Cv_Var< AutomCnstrCrate>	 s) {} 

    void    AddEpsSource( Cv_Var< AutomCnstrCrate>	 s) {} 

    bool    WriteDot( Cv_DotStream &strm) { return true; }
};

//_____________________________________________________________________________________________________________________________ 

typedef AutomCnstrCrate::Id                         AutomCnstrId;
typedef AutomCnstrCrate::Var                        AutomCnstrVar;

//_____________________________________________________________________________________________________________________________ 

struct  AutomSpurCnstr   : public AutomCnstr 
{ 
    std::vector< Sg_ChSet>          m_ChSets;
    std::vector< AutomCnstrVar>     m_Dests;
    std::set< AutomCnstrVar>        m_EpsDestIds; 
    std::set< AutomCnstrVar>        m_EpsFromSources;

public:
    AutomSpurCnstr( void)  
    {}
     

    const char		*GetName( void) const { return "Spur"; }

    void        AddEdge( const Sg_ChSet &chSet, AutomCnstrVar dest) 
    {
        m_ChSets.push_back( chSet);
        m_Dests.push_back( dest);
    }

    void        AddEpsDest( AutomCnstrVar s) 
    { 
        if ( this == s.GetEntry())
            return; 
        auto    res = m_EpsDestIds.insert( s); 
        if ( res.second)
            s->RaiseRef();  
        AutomCnstrVar   thisVar( this);
        s( [ thisVar]( auto k) {
            k->AddEpsSource( thisVar);
        });
        return;
    }
 
    void    AddEpsSource( AutomCnstrVar s) 
    { 
        if ( this == s.GetEntry())
            return; 
 
        auto    res = m_EpsFromSources.insert( s); 
        if ( res.second)
            s->RaiseRef(); 
        return;
    }

    bool    WriteDot( Cv_DotStream &strm)  
    {
        strm << 'R' << m_IPtr << " [ shape=ellipse color=cyan label= <<FONT> N" << GetId() << "<BR />" ; 
        strm << " </FONT>>];\n "; 

        for ( uint32_t k = 0; k < m_Dests.size(); ++k)
        {
            AutomCnstrVar		regex = m_Dests[ k];
            strm << 'R' << m_IPtr << " -> " << 'R' << regex->m_IPtr << " [ arrowhead=normal color=black label=<<FONT> ";  
            strm << Cv_Aid::XmlEncode(  m_ChSets[ k].ToString());
            strm << "</FONT>>] ; \n" ;  
        } 

        for ( auto it = m_EpsDestIds.begin(); it !=  m_EpsDestIds.end(); ++it) 
            strm << 'R' << m_IPtr << " -> " << 'R' << it->GetId() << " [ arrowhead=vee color=blue] ; \n"; 

        for ( auto it = m_EpsFromSources.begin(); it !=  m_EpsFromSources.end(); ++it)  
            strm << 'R' << it->GetId() << " -> " << 'R' << m_IPtr << " [ arrowhead=tee color=green] ; \n"; 
        return true;
    }
};

//_____________________________________________________________________________________________________________________________  
 
struct AutomCnstrRepos : public Cv_CrateRepos< AutomCnstrCrate>
{
    RExpRepos				*m_RexpRepos;
    
    AutomCnstrRepos(  RExpRepos *rexpRepos)
        : m_RexpRepos( rexpRepos)
    {}
    
    void    Proliferate( SynElem *elm, AutomCnstrVar start, AutomCnstrVar end)
    {
        return;   
    }

    void    Proliferate( CSetSynElem *csetElm, AutomCnstrVar start, AutomCnstrVar end)
    {
        start( [ csetElm, end]( auto k) {
            k->AddEdge( csetElm->m_Filt, end);
        });
        return;   
    }

    void    Proliferate( SeqSynElem *seqElm, AutomCnstrVar start, AutomCnstrVar end)
    {
          
        auto    right = end; 
        for ( uint32_t i = seqElm->m_SeqList.size(); i > 1; --i)
        {
            AutomSpurCnstr      *state =  Construct< AutomSpurCnstr>(); 
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

    void    Proliferate( AltSynElem *altElm, AutomCnstrVar start, AutomCnstrVar end)
    {
        for ( uint32_t i = 0; i < altElm->m_AltList.size(); ++i)
        {
            AutomSpurCnstr        *state =  Construct< AutomSpurCnstr>();
            start( [ state](  auto k) {
                k->AddEpsDest( state);
            }); 

            RExpCrate::Var      elemVar = m_RexpRepos->ToVar( altElm->m_AltList[ i]);

            elemVar( [ this, state, end](  auto k) {
                 Proliferate( k, state, end);
            });
        }
        return;   
    }
        
    void    Proliferate( RepeatSynElem *repElm, AutomCnstrVar start, AutomCnstrVar end)
    {
        uint32_t            nSeg  = ( repElm->m_Max == 0) ? ( repElm->m_Min +1) : repElm->m_Max; 
        AutomSpurCnstr      *fState = Construct< AutomSpurCnstr>();
        start( [ fState](  auto k) {
            k->AddEpsDest( fState);
        });
        AutomSpurCnstr                  *sState;
        AutomSpurCnstr                  *loopbackState;
        std::vector< AutomSpurCnstr *>  optSpinList;
        RExpCrate::Var      elemVar = m_RexpRepos->ToVar( repElm->m_Elem);
        for ( uint32_t i = 0; i < nSeg; ++i)
        { 
            sState = Construct< AutomSpurCnstr>();
            elemVar( [ this, fState, sState](  auto k) {
                Proliferate( k, fState, sState);
            });
            if (( repElm->m_Max && ( i >= repElm->m_Min)) || ( !repElm->m_Max && ( i == repElm->m_Min)))
                optSpinList.push_back( fState);
            if ( !repElm->m_Max && ( i == repElm->m_Min))
                loopbackState = fState;
            fState = sState;
        }
        if ( repElm->m_Max) 
            sState->AddEpsDest( end); 
        else
            sState->AddEpsDest( loopbackState);
        return;   
    }

    void    Process( void)
    {
        AutomSpurCnstr      *start =  Construct< AutomSpurCnstr>();
        AutomSpurCnstr      *end =  Construct< AutomSpurCnstr>();
        RExpCrate::Var      docVar = m_RexpRepos->ToVar( m_RexpRepos->m_RootId);
        docVar( [ this, start, end](  auto k) {
            Proliferate( k, start, end);
        });
        return;
    }
};

//_____________________________________________________________________________________________________________________________  
};

//_____________________________________________________________________________________________________________________________  

