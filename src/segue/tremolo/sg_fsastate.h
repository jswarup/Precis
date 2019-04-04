//  sg_fsastate.h _______________________________________________________________________________________________________________
#pragma once

#include    "cove/silo/cv_array.h"
#include    "cove/silo/cv_stack.h"
#include    "cove/silo/cv_repos.h"
#include 	"cove/barn/cv_ptrslot.h" 
#include 	"cove/barn/cv_compare.h" 
#include    "segue/tremolo/sg_filter.h"
#include    "cove/silo/cv_craterepos.h"
#include    "segue/timbre/sg_distrib.h"  

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{

struct    FsaRepos;
struct    FsaState;
struct    FsaElem;
struct    FsaSupState;
struct     FsaDfaState;
struct    FsaDfaCnstr;
struct    FsaDfaStateMap;
struct    FsaDfaStateMapCltn;

typedef Cv_Crate< FsaDfaState, FsaSupState, FsaElem, FsaState>              FsaCrate;  

//_____________________________________________________________________________________________________________________________ 

struct  Action
{
    std::vector< uint64_t>        m_Values;

    Action( void)
    {}
    

    Cv_CArr< uint64_t>      Tokens( void) { return m_Values.size() ? Cv_CArr< uint64_t>( &m_Values[ 0], uint32_t( m_Values.size())) : Cv_CArr< uint64_t>(); } 

    void         Push( uint64_t v) { m_Values.push_back( v); }
    void         Push( const Cv_CArr< uint64_t> &tokens)
    {   
        uint32_t        oldSz = uint32_t( m_Values.size());
        m_Values.resize( oldSz + tokens.Size());
        std::copy( tokens.Begin(), tokens.End(), &m_Values[ oldSz]);
    }

    int32_t         Compare( const Action &x2) const 
    {
        if ( m_Values.size() != x2.m_Values.size())
            return m_Values.size() < x2.m_Values.size() ? 1 : -1;
        const uint64_t       *arr1 = &m_Values[ 0];
        const uint64_t       *arr2 = &x2.m_Values[ 0];
        for ( uint32_t i = 0; i < m_Values.size(); ++i)
            if ( arr1[ i] != arr2[ i])
                return arr1[ i] < arr2[ i] ? 1 : -1;
        return 0;
    }
};

//_____________________________________________________________________________________________________________________________ 

struct FsaState  : public Cv_CrateEntry
{
    typedef  Id                 FsaId;

    typedef FilterRepos::Id     FiltId;
    typedef FilterCrate::Var    FiltVar;
    
public:
    ~FsaState( void) {}

    Cv_CArr< uint64_t>  Tokens( void) { return Cv_CArr< uint64_t>(); } 

    Cv_CArr< FiltId>    Filters( void) { return Cv_CArr< FiltId>(); }
    Cv_CArr< FsaId>     Dests( void) { return Cv_CArr< FsaId>(); }
    Cv_CArr< FsaId>     SubStates( void) { return Cv_CArr< FsaId>(); } 

    bool                WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm) { return false; }
};


//_____________________________________________________________________________________________________________________________ 

struct  FsaRepos  : public Cv_CrateRepos< FsaCrate>
{   
    typedef  Id                 FsaId;
    FsaId                       m_RootId;
    FilterRepos                 m_FilterRepos; 

    bool                WriteDot( Cv_DotStream &strm);
    bool                DumpDot( const char *path);

    bool                Dump( std::ostream &ostr)
    {
        m_FilterRepos.Dump( ostr);
        return true;
    }
};

//_____________________________________________________________________________________________________________________________ 

struct  FsaElemRepos  : public FsaRepos
{    
    std::vector< uint32_t>      m_RuleIdSzList;

    uint32_t            RuleIdFromState( uint32_t k) const 
    {
        auto        it = std::upper_bound( m_RuleIdSzList.begin(), m_RuleIdSzList.end(), k); 
        return uint32_t( std::distance(  m_RuleIdSzList.begin(), it));
    } 
};


//_____________________________________________________________________________________________________________________________ 

struct  FsaElem   : public FsaState, public Cv_Shared
{      
    Action                      *m_Action;
    std::vector< FiltId>        m_ChSets;
    std::vector< FsaId>         m_Dests;

    FsaElem( void)
        : m_Action( NULL)
    {}

    ~FsaElem( void)
    {
        if ( m_Action)
            delete m_Action;
    }

    void    AddEdge( FiltId chSet, FsaId  dest) 
    {
        m_ChSets.push_back( chSet);
        m_Dests.push_back( dest);
    } 

    Cv_CArr< uint64_t>          Tokens( void) { return m_Action ?  Cv_CArr< uint64_t>( &m_Action->m_Values[ 0], uint32_t( m_Action->m_Values.size())) : Cv_CArr< uint64_t>(); } 
    Cv_CArr< FsaId>             Dests( void) { return m_Dests.size() ? Cv_CArr< FsaId>( &m_Dests[ 0], uint32_t( m_Dests.size())) : Cv_CArr< FsaId>(); }  
    Cv_CArr< FiltId>            Filters( void) { return m_ChSets.size() ? Cv_CArr< FiltId>( &m_ChSets[ 0], uint32_t( m_ChSets.size())) : Cv_CArr< FiltId>(); } 
 

    bool                        WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm);
};

//_____________________________________________________________________________________________________________________________ 

};