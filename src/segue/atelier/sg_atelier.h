// sg_atelier.h ___________________________________________________________________________________________________________________
#pragma once


#include    "segue/timbre/sg_bitset.h"
#include    "segue/timbre/sg_partition.h"
#include    "segue/tremolo/sg_dfastate.h"
 
//_____________________________________________________________________________________________________________________________

using namespace Sg_RExp;

//_____________________________________________________________________________________________________________________________

struct Sg_Parapet
{
    FsaDfaState     *m_CurState; 
    
    Sg_Parapet( void)
        :  m_CurState( NULL) 
    {}

    Sg_Parapet( FsaDfaState *rootState)
        :  m_CurState( rootState) 
    {}
    
    bool    IsLoaded( void) const 
    {
        return !!m_CurState;
    }

    bool    Advance( FsaDfaRepos *dfaRepos, uint8_t chr)
    {
        DistribCrate::Var   dVar = dfaRepos->m_DistribRepos.ToVar( m_CurState->DistribId());
        uint8_t             img = dVar( [ chr]( auto k) { return k->Image( chr); }); 
        m_CurState = static_cast< FsaDfaState *>( dfaRepos->ToVar( m_CurState->Dests().At( img)).GetEntry());
        return !!m_CurState;
    }

    Cv_CArr< uint64_t>      Tokens( void) { return m_CurState->Tokens(); }
};

//_____________________________________________________________________________________________________________________________

struct Sg_Atlier
{ 
    FsaDfaRepos     *m_DfaRepos;
    Sg_Parapet      m_Parapet;        
    
    Sg_Atlier( FsaDfaRepos *dfaRepos)
        : m_DfaRepos( dfaRepos)
    {}

    bool        Play( uint8_t chr)
    {
        if ( !m_Parapet.IsLoaded())
        {        
            FsaDfaState     *rootDfaState = static_cast< FsaDfaState *>( m_DfaRepos->ToVar( m_DfaRepos->m_RootId).GetEntry());
            m_Parapet = Sg_Parapet( rootDfaState );
        }
        uint8_t     chrId = m_DfaRepos->m_DistribRepos.m_Base.Image( chr);
        if ( !m_Parapet.Advance( m_DfaRepos, chrId))
             return false;
        Cv_CArr< uint64_t>      tokens = m_Parapet.Tokens();

        for ( uint32_t i = 0; i < tokens.Size(); ++i)
            std::cout << tokens[ i] << "\n";
        return true;
    
    }   

    
};

//_____________________________________________________________________________________________________________________________
