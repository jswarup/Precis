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

    void    AddEpsDest( Cv_Var< AutomCnstrCrate>	 s) 
    {
    }
    void    AddEpsSource( Cv_Var< AutomCnstrCrate>	 s) 
    {
    }
};

//_____________________________________________________________________________________________________________________________ 

typedef AutomCnstrCrate::Id                         AutomCnstrId;
typedef AutomCnstrCrate::Var                        AutomCnstrVar;

//_____________________________________________________________________________________________________________________________ 

struct  AutomSpurCnstr   : public AutomCnstr 
{ 
    std::vector< Sg_ChSet>          m_ChSets;
    std::vector< AutomCnstrId>      m_Dests;
    std::set< AutomCnstrId>         m_EpsDestIds; 
    std::set< AutomCnstrId>         m_EpsFromSources;
public:
    AutomSpurCnstr( void)  
    {}
     
    void        AddEpsDest( AutomCnstrVar s) 
    { 
        if ( this == s.GetEntry())
            return; 
        auto    res = m_EpsDestIds.insert( s.GetId()); 
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
 
        auto    res = m_EpsFromSources.insert( s.GetId()); 
        if ( res.second)
            s->RaiseRef(); 
        return;
    }
};

//_____________________________________________________________________________________________________________________________  
 
struct AutomCnstrRepos : public Cv_CrateRepos< AutomCnstrCrate>
{
    RExpRepos				*m_RexpRepos;
    
    AutomCnstrRepos(  RExpRepos *rexpRepos)
        : m_RexpRepos( rexpRepos)
    {}
    
    void    Proliferate( SynElem *altElm, AutomCnstrVar start, AutomCnstrVar end)
    {
        return;   
    }

    void    Proliferate( AltSynElem *altElm, AutomCnstrVar start, AutomCnstrVar end)
    {
        for ( uint32_t i = 0; i < altElm->m_AltList.size(); ++i)
        {
            AutomCnstrVar        state =  new AutomSpurCnstr();
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

    void    Process( void)
    {
        AutomCnstrVar        start =  new AutomSpurCnstr();
        AutomCnstrVar        end =  new AutomSpurCnstr();
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

