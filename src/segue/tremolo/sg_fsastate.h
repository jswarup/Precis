//  sg_fsastate.h _______________________________________________________________________________________________________________
#pragma once

#include 	"cove/barn/cv_ptrslot.h" 
#include    "segue/tremolo/sg_filter.h"
#include    "cove/silo/cv_craterepos.h"

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{

struct    FsaRepos;
struct    FsaState;
struct    AutomElem;
struct    FsaCState;

typedef Cv_Crate< FsaCState, AutomElem, FsaState>                                                          FsaCrate;  

//_____________________________________________________________________________________________________________________________ 

struct FsaState  : public Cv_CrateEntry, public Cv_Shared
{
    typedef FsaCrate::Var       FsaVar;
    typedef FilterCrate::Var    FiltVar;

    uint32_t        SzToken( void) { return 0; }
    uint64_t        Token( uint32_t k) { return 0; }

    uint32_t        SzDest( void) { return 0; }

    FsaVar          Dest( FsaRepos *repos, uint32_t k) { return FsaVar(); }
    FiltVar         Filter( uint32_t k) { return FiltVar(); } 

    bool            WriteDot( Cv_DotStream &strm) { return false; }
};

//_____________________________________________________________________________________________________________________________ 

struct FsaCState  : public FsaState
{ 
}; 

//_____________________________________________________________________________________________________________________________ 

struct  FsaRepos  : public Cv_CrateRepos< FsaCrate>
{
    FilterRepos     m_FilterRepos;

    bool        WriteDot( Cv_DotStream &strm);
};
//_____________________________________________________________________________________________________________________________ 

};