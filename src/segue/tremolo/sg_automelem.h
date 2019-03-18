//  sg_automelem.h ___________________________________________________________________________________________________________________
#pragma once

#include 	"cove/barn/cv_ptrslot.h" 
#include    "segue/tremolo/sg_filter.h"
#include 	"cove/silo/cv_repos.h"
#include 	"cove/silo/cv_dotstream.h"
#include    "segue/timbre/sg_distrib.h"

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{
struct  Action
{
    uint64_t        m_Value;
    
    Action( uint64_t value)
        : m_Value( value)
    {}
};

//_____________________________________________________________________________________________________________________________ 

struct  AutomElem   : public Cv_ReposEntry, public Cv_Shared
{      
    Action                          *m_Action;
    std::vector< Sg_ChSet>         m_ChSets;
    std::vector< AutomElem *>      m_Dests;

    AutomElem( void)
        : m_Action( NULL)
    {}

    void        AddEdge( const Sg_ChSet &chSet, AutomElem *dest) 
    {
        m_ChSets.push_back( chSet);
        m_Dests.push_back( dest);
        dest->RaiseRef();
    } 

    bool        WriteDot( Cv_DotStream &strm);
};

//_____________________________________________________________________________________________________________________________ 

struct  AutomRepos  : public Cv_Repos< AutomElem>
{
    FilterRepos     m_FilterRepos;

    bool        WriteDot( Cv_DotStream &strm)  
    {
        for ( uint32_t i = 1; i < Size(); ++i)
        {
            AutomElem  *si = At( i);
            if (si)
                si->WriteDot( strm); 
        }
        return true;
    }
};

//_____________________________________________________________________________________________________________________________ 
};
