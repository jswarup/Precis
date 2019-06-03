// sg_atelier.h ___________________________________________________________________________________________________________________
#pragma once


#include    "segue/timbre/sg_bitset.h"
#include    "segue/timbre/sg_partition.h"
#include    "segue/tremolo/sg_dfastate.h"
#include    <array>
 
//_____________________________________________________________________________________________________________________________

using namespace Sg_RExp;

//_____________________________________________________________________________________________________________________________

struct Sg_DfaReposAtelier
{
    FsaDfaRepos     *m_DfaRepos;
    
    Sg_DfaReposAtelier( FsaDfaRepos *dfaRepos = NULL)
        : m_DfaRepos( dfaRepos)
    {}
    
    uint8_t                 ByteCode( uint8_t chr )   { return m_DfaRepos->m_DistribRepos.m_Base.Image( chr); }
     
    FsaCrate::Var           RootState( void) {  return m_DfaRepos->ToVar( m_DfaRepos->m_RootId); } 

    FsaCrate::Var           DfaTransition( FsaState *state, uint8_t chrId) 
    {  
        FsaDfaState         *dfaState = static_cast< FsaDfaState *>( state);
        DistribCrate::Var   dVar = m_DfaRepos->m_DistribRepos.ToVar( dfaState->DistribId());
        uint8_t             img = dVar( [ chrId]( auto k) { return k->Image( chrId); }); 
        return m_DfaRepos->ToVar( dfaState->Dests().At( img));
    }

    FsaCrate::Var           DfaUniXTransition( FsaState *state, uint8_t chrId) 
    {  
        FsaDfaUniXState     *dfaState = static_cast< FsaDfaUniXState *>( state); 
        if ( chrId == dfaState->m_Byte)
            return m_DfaRepos->ToVar(  dfaState->m_Dest);
        return FsaCrate::Var();
    }
}; 

//_____________________________________________________________________________________________________________________________

struct Sg_DfaBlossomAtelier
{
    FsaDfaRepos::Blossom    m_DfaBlossom;
    DistribRepos::Blossom   m_Distribs;
    FsaRepos::Blossom       m_States;

    Sg_DfaBlossomAtelier( void *dfaImage)
        : m_DfaBlossom( dfaImage), m_Distribs( m_DfaBlossom.Distribs()), m_States( m_DfaBlossom.States())
    {}
 
    uint8_t                 ByteCode( uint8_t chr )  { return m_Distribs.Base()->Image( chr); }

    FsaCrate::Var           RootState( void)  {  return m_States.ToVar( m_DfaBlossom.RootId()); } 
    FsaCrate::Var           DfaTransition( FsaState *state, uint8_t chrId) 
    {  
        FsaDfaState         *dfaState = static_cast< FsaDfaState *>( state);
        DistribCrate::Var   dVar = m_Distribs.ToVar( dfaState->DistribId());
        uint8_t             img = dVar( [ chrId]( auto k) { return k->Image( chrId); }); 
        return  m_States.ToVar( dfaState->Dests().At( img));
    }
    
    FsaCrate::Var           DfaUniXTransition( FsaState *state, uint8_t chrId) 
    {  
        FsaDfaUniXState     *dfaState = static_cast< FsaDfaUniXState *>( state); 
        if ( chrId == dfaState->m_Byte)
            return m_States.ToVar(  dfaState->m_Dest);
        return FsaCrate::Var();
    }
};


//_____________________________________________________________________________________________________________________________

struct Sg_Parapet
{
    FsaClip         m_CurState;
    uint64_t        m_Start; 
    
    Sg_Parapet( void)
        :  m_Start( 0)
    {}

    void        Load( const FsaCrate::Var &rootState, uint64_t start)
    {
        m_CurState = rootState;
        m_Start = start; 
    }
    
    bool        IsLoaded( void) const  { return !!m_CurState; }

    uint64_t    Start( void) const  { return m_Start; } 

template < typename Atelier>
    bool        Advance( Atelier *dfaAtelier, uint8_t chrId)
    {
        m_CurState( [ this, dfaAtelier, chrId]( auto k) { 
                switch ( k->GetType())
                { 
                    case FsaCrate::template TypeOf< FsaDfaState>() : m_CurState =  dfaAtelier->DfaTransition( k, chrId); break;
                    case FsaCrate::template TypeOf< FsaDfaUniXState>() : m_CurState =  dfaAtelier->DfaUniXTransition( k, chrId); break;
                    default : m_CurState = FsaCrate::Var(); break;
                }
                return true;
            });
        return !!m_CurState;
    } 

    Cv_CArr< uint64_t>      Tokens( void) {  return m_CurState.Tokens(); }
};

//_____________________________________________________________________________________________________________________________

struct Sg_MatchData
{
    uint64_t                m_Start;
    uint32_t                m_Len;
    uint64_t                m_Token;

    Sg_MatchData( void)
        : m_Start( 0), m_Len( 0), m_Token( 0)
    {}

    Sg_MatchData( uint64_t start, uint32_t len, uint64_t token)
        : m_Start( start), m_Len( len), m_Token( token)
    {}

    friend std::ostream &operator<<( std::ostream &strm, const Sg_MatchData &md)
    {
        strm << md.m_Start << " " << md.m_Len << " " <<  md.m_Token << "\n";
        return strm;
    }
};

//_____________________________________________________________________________________________________________________________

template < uint32_t Sz, typename TokenGram>
struct Sg_Bulwark
{ 
     
    uint64_t                                    m_Curr;    
    std::array< Sg_Parapet, Sz>                 m_Parapets; 
    Sg_Bitset< Sz>                              m_Allocbits;
    TokenGram                                   *m_TokenSet;
    uint32_t                                    m_PickInd; 

    Sg_Bulwark( void)
        : m_Curr( 0), m_TokenSet( NULL), m_PickInd( 0)
    {}
     
    
    void            DumpTokens( Sg_Parapet  *parapet)
    { 
        Cv_CArr< uint64_t>      tokens = parapet->Tokens(); 
        for ( uint32_t i = 0; i < tokens.Size(); ++i)
            m_TokenSet->Append( Sg_MatchData( parapet->Start(), uint32_t( m_Curr -parapet->Start()), tokens[ i]));
        return;
    }

template < typename Atelier>
    bool    Play( Atelier *dfaAtelier, uint8_t chrId)
    {   
        Sg_Parapet      *curent = &m_Parapets[ m_PickInd];  
        curent->Load( dfaAtelier->RootState(), m_Curr); 
        m_Allocbits.Set( m_PickInd, true);   
 
        Sg_Bitset< Sz>          allocbits; 
        m_PickInd = CV_UINT32_MAX;
        m_Allocbits.ForAllTrue( [this, dfaAtelier]( uint32_t ind, uint8_t chrId, Sg_Bitset< Sz> *allocbits)
            {
                Sg_Parapet      *parapet = &m_Parapets[ ind];
                if ( !parapet->Advance( dfaAtelier, chrId))
                {   
                    m_PickInd = ind;
                    return;
                }
                allocbits->Set( ind, true); 
                if ( m_TokenSet)
                    DumpTokens( &m_Parapets[ ind]);
                return;
            }, chrId, &allocbits);  
        if ( m_PickInd == CV_UINT32_MAX)
            m_PickInd = allocbits.Index( false);
        ++m_Curr;
        m_Allocbits = allocbits;
        return true; 
    }    
};
 
//_____________________________________________________________________________________________________________________________
