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
        return m_DfaRepos->ToVar( dfaState->DestAt( img));
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
    {
        m_DfaBlossom.SauteStates();
    }
 
    uint8_t                 ByteCode( uint8_t chr )  { return m_Distribs.Base()->Image( chr); }

    FsaCrate::Var           RootState( void)  {  return m_States.ToVar( m_DfaBlossom.RootId()); } 
    FsaCrate::Var           DfaTransition( FsaState *state, uint8_t chrId) 
    {  
        FsaDfaState         *dfaState = static_cast< FsaDfaState *>( state);
        DistribCrate::Var   dVar = m_Distribs.VarId( dfaState->DistribId());
        uint8_t             img = dVar( [ chrId]( auto k) { return k->Image( chrId); }); 
        return  m_States.VarId( dfaState->DestAt( img));
    }
    
    FsaCrate::Var           DfaUniXTransition( FsaState *state, uint8_t chrId) 
    {  
        FsaDfaUniXState     *dfaState = static_cast< FsaDfaUniXState *>( state); 
        if ( chrId == dfaState->m_Byte)
            return m_States.VarId(  dfaState->m_Dest);
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


    uint16_t        SzTokens( void)
    {
        return m_CurState( [ this]( auto k) { 
                switch ( k->GetType())
                { 
                    case FsaCrate::template TypeOf< FsaDfaState>() : return static_cast< FsaDfaState*>( static_cast< FsaState *>( k))->SzTokens();
                    case FsaCrate::template TypeOf< FsaDfaUniXState>() : 
                    default : return uint16_t( 0);
                }
            });
        
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

template < typename Atelier, typename TokenGram>
struct Sg_Bulwark
{  
    Atelier                                     *m_DfaAtelier;
    uint64_t                                    m_Curr;    
    FsaCrate::Var                               m_Root;
    uint32_t                                    m_PickInd; 
    std::array< Sg_Parapet, 64>                 m_Parapets; 
    uint64_t                                    m_Allocbits;
    TokenGram                                   *m_TokenSet;
    
    Sg_Bulwark( void)
        : m_DfaAtelier( NULL), m_Curr( 0), m_TokenSet( NULL), m_PickInd( 0), m_Allocbits( 0)
    {} 

    void    Setup( Atelier *dfaAtelier)
    {
        m_DfaAtelier = dfaAtelier;
        m_Root = dfaAtelier->RootState();
    }
        
    void            DumpTokens( Sg_Parapet  *parapet)
    { 
        Cv_CArr< uint64_t>      tokens = parapet->Tokens(); 
        for ( uint32_t i = 0; i < tokens.Size(); ++i)
            m_TokenSet->Append( Sg_MatchData( parapet->Start(), uint32_t( m_Curr -parapet->Start()), tokens[ i]));
    }
    
    bool    LoadRoot( void)
    {
        if ( m_PickInd == CV_UINT32_MAX)
            return false;

        Sg_Parapet      *curent = &m_Parapets[ m_PickInd];  
        curent->Load( m_Root, m_Curr); 
        m_Allocbits = ( m_Allocbits | ( uint64_t( 1) << m_PickInd));   
        m_PickInd = CV_UINT32_MAX;
        return true;
    }

    bool    Play( uint8_t chrId)
    {    
        uint64_t    allocbits = 0;
        for ( uint32_t ind = 0; m_Allocbits; ind++, m_Allocbits >>= 1)  
            if ( m_Allocbits & 1)
            {
                Sg_Parapet      *parapet = &m_Parapets[ ind];
                if ( !parapet->Advance( m_DfaAtelier, chrId)) 
                    m_PickInd = ind;   
                else
                {
                    allocbits = ( uint64_t( 1) << ind) | allocbits;
                    if ( m_TokenSet && parapet->SzTokens())
                        DumpTokens( parapet); 
                }
            } 
        m_Allocbits = allocbits;
        ++m_Curr; 
        if ( m_PickInd != CV_UINT32_MAX)
            return true;
    
        uint64_t    freeBits = ~allocbits;
        for ( uint32_t j = 0; freeBits; j++, freeBits >>= 1)  
            if ( freeBits & 1)
            {
                m_PickInd = j; 
                freeBits = 0;
            }
        return true; 
    }  
 
};
 
//_____________________________________________________________________________________________________________________________
/*
template < typename Atelier, typename TokenGram>
struct Sg_Bastion
{  
    enum { SzCol = 8};
    Atelier                                         *m_DfaAtelier;   
    TokenGram                                       *m_TokenSet;
    FsaCrate::Var                                   m_Root;
    std::array< uint64_t, SzCol>                    m_Curr; 
    std::array< uint32_t, SzCol>                    m_PickInd; 
    std::array< uint64_t, SzCol>                    m_Allocbits;
    std::array< std::array< Sg_Parapet, 64>, SzCol> m_Parapets; 

    Sg_Bastion( void)
        : m_DfaAtelier( NULL), m_TokenSet( NULL)
    {
        m_Curr.fill( 0);
        m_PickInd.fill( 0);
        m_Allocbits.fill( 0);
    } 

    void    Setup( Atelier *dfaAtelier)
    {
        m_DfaAtelier = dfaAtelier;
        m_Root = dfaAtelier->RootState();
    }

    void            DumpTokens( Sg_Parapet  *parapet)
    { 
        Cv_CArr< uint64_t>      tokens = parapet->Tokens(); 
        for ( uint32_t i = 0; i < tokens.Size(); ++i)
            m_TokenSet->Append( Sg_MatchData( parapet->Start(), uint32_t( m_Curr -parapet->Start()), tokens[ i]));
    }

    bool    LoadRoot( void)
    {
        if ( m_PickInd == CV_UINT32_MAX)
            return false;
    
        for ( uint32_t c = 0; c < SzCol; ++c)
            m_Parapets[ c][ m_PickInd[ c]].Load( m_Root, m_Curr[ c]); 
        for ( uint32_t c = 0; c < SzCol; ++c)
            m_Allocbits[ c] = ( m_Allocbits[ c] | ( uint64_t( 1) << m_PickInd[ c]));   
        for ( uint32_t c = 0; c < SzCol; ++c)
            m_PickInd[ c] = CV_UINT32_MAX;
        return true;
    }

    bool    Play( uint8_t chrId)
    {    
        std::array< uint64_t, SzCol>    allocbits = 0;  
        for ( uint32_t ind = 0; ind < 64; ind++)  
        {
            if ( m_Allocbits[ c] & 1)
            { 
                if ( !m_Parapets[ ind].Advance( m_DfaAtelier, chrId)) 
                    m_PickInd = ind;   
                else
                {
                    allocbits = ( uint64_t( 1) << ind) | allocbits;
                    if ( m_TokenSet && m_Parapets[ ind].SzTokens())
                        DumpTokens( &m_Parapets[ ind].); 
                }
            }

            for ( uint32_t c = 0; c < SzCol; ++c)
                m_Allocbits[ c] >>= 1
        }
        for ( uint32_t c = 0; c < SzCol; ++c)
            m_Allocbits[ c] = allocbits[ c];
        for ( uint32_t c = 0; c < SzCol; ++c)
            ++m_Curr[ c]; 
        if ( m_PickInd[ c] = CV_UINT32_MAX)
        {
            uint64_t    freeBits = ~allocbits;
            for ( uint32_t j = 0; freeBits; j++, freeBits >>= 1)  
                if ( freeBits & 1)
                {
                    m_PickInd = j; 
                    freeBits = 0;
                }
        }
        return true; 
    }  

};
*/
//_____________________________________________________________________________________________________________________________
