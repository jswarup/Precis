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

    Cv_CArr< FiltVar>           Filters( void) { return Cv_CArr< FiltVar>(); }
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


    Cv_CArr< FsaVar>            SubStates( void) { return m_SubStates.size() ? Cv_CArr< FsaVar>( &m_SubStates[ 0], uint32_t( m_SubStates.size())) : Cv_CArr< FsaVar>(); } 
    
    std::vector< Sg_ChSet>      RefineCharDistrib(  void);
}; 

//_____________________________________________________________________________________________________________________________ 

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
    std::vector< Sg_ChSet>          m_ChSets;
    std::vector< FsaVar>            m_Dests;

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

    Cv_CArr< uint64_t>      Tokens( void) { return m_Action ? Cv_CArr< uint64_t>() : Cv_CArr< uint64_t>( &m_Action->m_Value, 1); } 
    Cv_CArr< FsaVar>        Dests( void) { return m_Dests.size() ? Cv_CArr< FsaVar>( &m_Dests[ 0], uint32_t( m_Dests.size())) : Cv_CArr< FsaVar>(); }  

    bool        WriteDot( Cv_DotStream &strm);
};

//_____________________________________________________________________________________________________________________________ 

struct FsaVarA : public FsaCrate::Var
{
    typedef FsaCrate::Var       FsaVar;
    typedef FilterCrate::Var    FiltVar;

    Cv_CArr< uint64_t>          Tokens( void) { return SELF( [this]( auto k) { return k->Tokens(); }); }

    Cv_CArr< FsaVar>            Dests( void) { return SELF( [this]( auto k) { return k->Dests(); }); }
    Cv_CArr< FiltVar>           Filters( void) { return SELF( [this]( auto k) { return k->Filters(); }); } 
    Cv_CArr< FsaVar>            SubStates( void) { return SELF( [this]( auto k) { return k->SubStates(); }); }
};

//_____________________________________________________________________________________________________________________________ 

};