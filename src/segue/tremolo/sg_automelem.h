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
