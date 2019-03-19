//  sg_fsastate.cpp _____________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/tremolo/sg_fsastate.h"
#include    "segue/tremolo/sg_automelem.h"

using namespace Sg_RExp;

//_____________________________________________________________________________________________________________________________

bool        FsaRepos::WriteDot( Cv_DotStream &strm)
{
    for ( uint32_t i = 1; i < Size(); ++i)
    {
        Var     si = Get( i);
        if (si)
            si( [&strm]( auto k) { k->WriteDot( strm); });
    }
    return true;
}

//_____________________________________________________________________________________________________________________________
 