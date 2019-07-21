//  sg_fsastate.h _______________________________________________________________________________________________________________
#pragma once

#include    "cove/silo/cv_array.h"
#include    "cove/silo/cv_stack.h"
#include    "cove/silo/cv_repos.h"
#include 	"cove/barn/cv_ptrslot.h" 
#include 	"cove/barn/cv_compare.h" 
#include    "segue/colophon/sg_filter.h"
#include    "cove/silo/cv_craterepos.h"
#include    "segue/epigraph/sg_distrib.h"  

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{

struct    FsaRepos;
struct    FsaState;
struct    FsaElem;
struct    FsaSupState;
struct     FsaDfaState;
template < uint32_t Sz>
struct    FsaDfaByteState;
struct    FsaDfaXByteState;
struct    FsaDfaCnstr;
struct    FsaRuleLump;
struct    FsaRuleLumpRepos;

typedef Cv_Crate< FsaDfaXByteState, FsaDfaByteState< 8>, FsaDfaByteState< 7>, FsaDfaByteState< 6>, FsaDfaByteState< 5>, FsaDfaByteState< 4>, FsaDfaByteState< 3>, FsaDfaByteState< 2>, FsaDfaByteState< 1>, FsaDfaState, FsaSupState, FsaElem, FsaState>              FsaCrate;  

//_____________________________________________________________________________________________________________________________ 

struct  Action
{
    std::vector< uint64_t>        m_Values;

    Action( void)
    {}
    

    Cv_Seq< uint64_t>      Tokens( void) { return m_Values.size() ? Cv_Seq< uint64_t>( &m_Values[ 0], uint32_t( m_Values.size())) : Cv_Seq< uint64_t>(); } 

    void         Push( uint64_t v) { m_Values.push_back( v); }
    void         Push( const Cv_Seq< uint64_t> &tokens)
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
    typedef FsaCrate            Crate;
    typedef  Id                 FsaId;
    typedef FilterRepos::Id     FiltId;
    typedef FilterCrate::Var    FiltVar; 
 
    struct Cask : public Cv_SerializeUtils 
    {      
        typedef FsaState           Type;
        typedef FsaState          ContentType;  

        static uint32_t         Spread( ContentType *obj) { return sizeof( *obj); }

        static ContentType      Encase( Cv_Spritz *spritz, const FsaState &obj)
        { 
            return obj;
        } 
    };

public:
    ~FsaState( void) {}

    Cv_Seq< uint64_t>  Tokens( void) { return Cv_Seq< uint64_t>(); } 

    Cv_Seq< FiltId>    Filters( void) { return Cv_Seq< FiltId>(); }
    Cv_Seq< FsaId>     Dests( void) { return Cv_Seq< FsaId>(); }
    Cv_Seq< FsaId>     SubStates( void) { return Cv_Seq< FsaId>(); } 

    bool                CleanupDestIds( FsaRepos *dfaRepos) { return false; }
    bool                WriteDot( Id id, void *, Cv_DotStream &strm) { return false; } 

    FsaId               DfaTransition( FsaRepos *dfaRepos, uint8_t chr) { return FsaId(); }
    bool                DoSaute( void *) { return false; }

    FsaId               Eval( void *atelier, uint8_t chrId) { return FsaId(); } 
};


//_____________________________________________________________________________________________________________________________ 

struct  FsaRepos  : public Cv_CrateRepos< FsaCrate>
{   
    typedef  Id         FsaId;

    bool                WriteDot( Cv_DotStream &strm);  
     
    struct Blossom
    {
        typedef typename Cask::ContentType    ContentType; 
        typedef typename Cask::SubContent    SubContent; 

        ContentType                         *m_Root;
        Cv_Seq< SubContent>                 m_Elems;
        Cv_Seq< TypeStor>                   m_Types;
        bool                                m_SauteFLg;

        Blossom( uint8_t *arr)
            : m_Root( ( ContentType *) arr), m_Elems( m_Root->Elems()), m_Types( m_Root->Types()), m_SauteFLg( false)
        {} 

        uint32_t        Size( void) const { return m_Elems.Size(); }
        Id			    GetId( uint32_t k) { return Id( k, m_Types[ k]); } 

        Var             VarAt( uint32_t k) { return Var( m_Elems[ k].Value(),  m_Types[ k]) ; }

        Var             ToVar( const Id &id)  
        {          
            if ( !id.IsValid())
                return Var();
            return Var(  m_Elems[ id.GetId()].Value(), id.GetType());
        }
 
        void    ConvertIdToVarId( Id *pId)
        {
            if ( !pId->IsValid())
                return;
            Var         var =     ToVar( *pId);
            uint64_t    ptrDist = uint64_t( (( uint8_t *) var.GetEntry()) - (( uint8_t *) m_Root));
            pId->SetId( ptrDist);
            Var         var1 = VarId( *pId);
            CV_ERROR_ASSERT( var.GetEntry() == var1.GetEntry())
            CV_ERROR_ASSERT( var.GetType() == var1.GetType())
            return;
        }

        Var    VarId( const Id &id) const
        {
            return  id.IsValid() ? Var( ( Entry *) ((( uint8_t *) m_Root) + id.GetId()), id.GetType()) :  Var();
        } 
    };
};

//_____________________________________________________________________________________________________________________________ 

struct  FsaElemRepos  : public FsaRepos
{    
    std::vector< uint32_t>      m_RuleIdSzList;
    FilterRepos                 m_FilterRepos; 
    FsaId                       m_RootId;

    uint32_t            RuleIdFromState( uint32_t k) const 
    {
        auto        it = std::upper_bound( m_RuleIdSzList.begin(), m_RuleIdSzList.end(), k); 
        return uint32_t( std::distance(  m_RuleIdSzList.begin(), it));
    } 


    bool                Dump( std::ostream &ostr)
    {
        m_FilterRepos.Dump( ostr);
        return true;
    }

    bool                DumpStats( std::ostream &ostr)
    {
        FsaRepos::DumpStats( ostr);
        m_FilterRepos.DumpStats( ostr);
        return true;
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

    Cv_Seq< uint64_t>          Tokens( void) { return m_Action ?  Cv_Seq< uint64_t>( &m_Action->m_Values[ 0], uint32_t( m_Action->m_Values.size())) : Cv_Seq< uint64_t>(); } 
    Cv_Seq< FsaId>             Dests( void) { return m_Dests.size() ? Cv_Seq< FsaId>( &m_Dests[ 0], uint32_t( m_Dests.size())) : Cv_Seq< FsaId>(); }  
    Cv_Seq< FiltId>            Filters( void) { return m_ChSets.size() ? Cv_Seq< FiltId>( &m_ChSets[ 0], uint32_t( m_ChSets.size())) : Cv_Seq< FiltId>(); } 
 

    bool                        WriteDot( Id id, FsaRepos *fsaRepos, Cv_DotStream &strm);

    bool                        WriteDot( Id id, void *, Cv_DotStream &strm) { return false; }
};

//_____________________________________________________________________________________________________________________________ 

};