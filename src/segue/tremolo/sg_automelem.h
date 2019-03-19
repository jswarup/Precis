//  sg_automelem.h ___________________________________________________________________________________________________________________
#pragma once

#include 	"cove/barn/cv_ptrslot.h" 
#include    "segue/tremolo/sg_filter.h"
#include 	"cove/silo/cv_repos.h"
#include 	"cove/silo/cv_dotstream.h"
#include    "segue/timbre/sg_distrib.h"
#include    "segue/tremolo/sg_fsastate.h"

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

struct  AutomElem   : public FsaState
{      
    Action                          *m_Action;
    std::vector< Sg_ChSet>         m_ChSets;
    std::vector< AutomElem *>      m_Dests;

    AutomElem( void)
        : m_Action( NULL)
    {}
    
    ~AutomElem( void)
    {
        if ( m_Action)
            delete m_Action;
    }

    void            AddEdge( const Sg_ChSet &chSet, AutomElem *dest) 
    {
        m_ChSets.push_back( chSet);
        m_Dests.push_back( dest);
        dest->RaiseRef();
    } 
    
    uint32_t        SzToken( void) { return m_Action ? 1 : 0; }
    uint64_t        Token( uint32_t k) { return ( m_Action && !k) ? m_Action->m_Value : 0; }
    FsaVar          Dest( FsaRepos *repos, uint32_t k) { return FsaVar( m_Dests[ k], FsaRepos::Crate::TypeOf< AutomElem>()); }

    bool        WriteDot( Cv_DotStream &strm);
};

//_____________________________________________________________________________________________________________________________ 

struct  AutomRepos  : public FsaRepos
{
    FilterRepos     m_FilterRepos;

    bool        WriteDot( Cv_DotStream &strm)  
    {
        for ( uint32_t i = 1; i < Size(); ++i)
        {
            Var     si = Get( i);
            if (si)
                si( [&strm]( auto k) { k->WriteDot( strm); });
        }
        return true;
    }
};

//_____________________________________________________________________________________________________________________________ 
};
