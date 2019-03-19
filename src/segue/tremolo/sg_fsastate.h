//  sg_fsastate.h _______________________________________________________________________________________________________________
#pragma once

#include    "cove/silo/cv_array.h"
#include 	"cove/barn/cv_ptrslot.h" 
#include    "segue/tremolo/sg_filter.h"
#include    "cove/silo/cv_craterepos.h"

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{

struct    FsaRepos;
struct    FsaState;
struct    AutomElem;
struct    FsaSupState;

typedef Cv_Crate< FsaSupState, AutomElem, FsaState>                                                          FsaCrate;  

//_____________________________________________________________________________________________________________________________ 

struct FsaState  : public Cv_CrateEntry, public Cv_Shared
{
    typedef FsaCrate::Var       FsaVar;
    typedef FilterCrate::Var    FiltVar;

    Cv_CArr< uint64_t>          Tokens( void) { return Cv_CArr< uint64_t>(); } 

    Cv_CArr< FiltVar>           Filter( void) { return Cv_CArr< FiltVar>(); }
    Cv_CArr< FsaVar>            Dests( void) { return Cv_CArr< FsaVar>(); }
    Cv_CArr< FsaVar>            SubStates( void) { return Cv_CArr< FsaVar>(); } 

    bool            WriteDot( Cv_DotStream &strm) { return false; }
};

//_____________________________________________________________________________________________________________________________ 

struct  FsaRepos  : public Cv_CrateRepos< FsaCrate>
{
    FilterRepos     m_FilterRepos;

    bool        WriteDot( Cv_DotStream &strm);
};

//_____________________________________________________________________________________________________________________________ 

struct FsaSupState  : public FsaState
{ 
    std::vector< FsaVar>     m_SubStates;


    Cv_CArr< FsaVar>            SubStates( void) { return Cv_CArr< FsaVar>( &m_SubStates[ 0], m_SubStates.size()); } 
    
    std::vector< Sg_ChSet>      RefineCharDistrib(  void);
}; 

//_____________________________________________________________________________________________________________________________ 

};