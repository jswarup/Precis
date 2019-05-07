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
    uint64_t        m_Start; 
    
    Sg_Parapet( void)
        :  m_CurState( NULL), m_Start( 0)
    {}

    void        Load( FsaDfaState *rootState, uint64_t start)
    {
        m_CurState = rootState;
        m_Start = start; 
    }
    
    bool        IsLoaded( void) const  { return !!m_CurState; }

    uint64_t    Start( void) const  { return m_Start; }

    bool        Advance( FsaDfaRepos *dfaRepos, uint8_t chr)
    {
        DistribCrate::Var   dVar = dfaRepos->m_DistribRepos.ToVar( m_CurState->DistribId());
        uint8_t             img = dVar( [ chr]( auto k) { return k->Image( chr); }); 
        m_CurState = static_cast< FsaDfaState *>( dfaRepos->ToVar( m_CurState->Dests().At( img)).GetEntry());
        return !!m_CurState;
    }

    Cv_CArr< uint64_t>      Tokens( void) { return m_CurState->Tokens(); }
};

//_____________________________________________________________________________________________________________________________

struct Sg_Rampart
{ 
    FsaDfaRepos     *m_DfaRepos;
    Sg_Parapet      m_Parapet;   
    uint64_t        m_Curr;     
    
    Sg_Rampart( void)
        : m_DfaRepos( NULL), m_Curr( 0)
    {}
    
    void        SetDfaRepos( FsaDfaRepos *dfaRepos) { m_DfaRepos = dfaRepos; }
    
    bool        Play( uint8_t chr)
    {
        uint8_t     chrId = m_DfaRepos->m_DistribRepos.m_Base.Image( chr);
        if ( !m_Parapet.IsLoaded())
        {        
            FsaDfaState     *rootDfaState = static_cast< FsaDfaState *>( m_DfaRepos->ToVar( m_DfaRepos->m_RootId).GetEntry());
            m_Parapet.Load( rootDfaState, m_Curr);
        }
        ++m_Curr;
        if ( !m_Parapet.Advance( m_DfaRepos, chrId))
             return false;
        Cv_CArr< uint64_t>      tokens = m_Parapet.Tokens();

        for ( uint32_t i = 0; i < tokens.Size(); ++i)
            std::cout << m_Parapet.Start() << " " << ( m_Curr -m_Parapet.Start()) << " " <<  tokens[ i] << "\n";
        return true; 
    }    
};

//_____________________________________________________________________________________________________________________________

struct Sg_Bastion
{
    FsaDfaRepos                 *m_DfaRepos;
    std::vector< Sg_Rampart>    m_Ramparts;
    
    Sg_Bastion( void)
        : m_DfaRepos( NULL)
    {}

    void        SetDfaRepos( FsaDfaRepos *dfaRepos) { m_DfaRepos = dfaRepos; }

template < typename Wharf>
    uint32_t    Play( Wharf *wharf)
    {
        uint32_t        szBurst = wharf->Size(); 
        uint32_t        dInd = 0;
        for ( ; dInd < szBurst;  dInd++)
        {
            Datagram    *datagram = wharf->Get( dInd); 
            for ( uint32_t k = 0; k < datagram->SzFill(); ++k)
            {
                uint8_t     chr = datagram->At( k);
                m_Atelier.Play( chr);
            }
        }
        return dInd;
    }
};

//_____________________________________________________________________________________________________________________________
