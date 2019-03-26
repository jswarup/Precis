//  sg_fsastate.h _______________________________________________________________________________________________________________
#pragma once

#include    "cove/silo/cv_array.h"
#include    "cove/silo/cv_stack.h"
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
struct    FsaDfaState;
struct    FsaDfaCnstr;

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
};

//_____________________________________________________________________________________________________________________________ 

struct FsaState  : public Cv_CrateEntry, public Cv_Shared
{
    typedef  Id                 FsaId;

    typedef FilterRepos::Id     FiltId;
    typedef FilterCrate::Var    FiltVar;
    
public:
    virtual     ~FsaState( void){}

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
};

//_____________________________________________________________________________________________________________________________ 

struct FsaSupState  : public FsaState
{   
    std::vector< FsaId>             m_SubStates;  

    ~FsaSupState( void)
    {}

    int32_t     Compare( const FsaSupState &x2) const 
    {
        if ( m_SubStates.size() != x2.m_SubStates.size())
            return m_SubStates.size() < x2.m_SubStates.size() ? 1 : -1;
        const FsaId       *arr1 = &m_SubStates[ 0];
        const FsaId       *arr2 = &x2.m_SubStates[ 0];
        for ( uint32_t i = 0; i < m_SubStates.size(); ++i)
            if ( arr1[ i] != arr2[ i])
                return arr1[ i] < arr2[ i] ? 1 : -1;
        return 0;
    } 

    Cv_CArr< FsaId>         SubStates( void) { return m_SubStates.size() ? Cv_CArr< FsaId>( &m_SubStates[ 0], uint32_t( m_SubStates.size())) : Cv_CArr< FsaId>(); } 
    
    Sg_CharDistrib          RefineCharDistrib( FsaRepos *fsaRepos);
    FsaDfaState             *DoConstructTransisition( FsaDfaCnstr *dfaCnstr);
    bool                    WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm);
}; 


//_____________________________________________________________________________________________________________________________ 

struct FsaDfaState  : public FsaState
{ 
    std::vector< FsaId>     m_Dests; 
    Action                  *m_Action;
    
    FsaDfaState( void)
        : m_Action( NULL)
    {}

    ~FsaDfaState( void)
    {
        if ( m_Action)
            delete m_Action;
    }

    Cv_CArr< FsaId>         Dests( void) { return m_Dests.size() ? Cv_CArr< FsaId>( &m_Dests[ 0], uint32_t( m_Dests.size())) : Cv_CArr< FsaId>(); } 
 
    
    bool                    WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm);
    void                    ExtractActionFrom( const Cv_CArr< uint64_t> &tokens)
    {
        if ( !tokens.Size())
            return;
        if ( !m_Action)
            m_Action = new Action();
        m_Action->Push( tokens);
        return;
    }
}; 


//_____________________________________________________________________________________________________________________________ 

struct  FsaElem   : public FsaState
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

    void            AddEdge( FiltId chSet, FsaId  dest) 
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

struct FsaClip  : public FsaCrate::Var
{
    typedef FsaCrate::Var       FsaVar;
    typedef FsaRepos::Id        FsaId;
    typedef FilterCrate::Var    FiltVar;
    typedef FilterRepos::Id     FiltId;
    
    FsaClip( const FsaVar &v)
        : FsaVar( v)
    {} 

    Cv_CArr< uint64_t>          Tokens( void) { return SELF( [this]( auto k) { return k->Tokens(); }); }

    Cv_CArr< FsaId>             Dests( void) { return SELF( [this]( auto k) { return k->Dests(); }); }
    Cv_CArr< FiltId>            Filters( void) { return SELF( [this]( auto k) { return k->Filters(); }); } 
    Cv_CArr< FsaId>             SubStates( void) { return SELF( [this]( auto k) { return k->SubStates(); }); }
};

//_____________________________________________________________________________________________________________________________ 

struct  FsaDfaCnstr 
{
    typedef std::map< FsaSupState*, FsaDfaState *, Cv_TPtrLess< void> >  SupDfaMap;

    typedef FsaRepos::Id                            FsaId;
    
     
    FsaRepos                    *m_FsaRepos; 
    SupDfaMap                   m_SupDfaMap;                  
    std::vector< FsaSupState *> m_FsaStk;
    
    FsaDfaCnstr( FsaRepos *fsaRepos)
        : m_FsaRepos( fsaRepos)
    {}
    
    void    SubsetConstruction( void);
};

//_____________________________________________________________________________________________________________________________ 

};