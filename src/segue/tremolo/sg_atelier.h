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


    DistribCrate::Var       FetchDistib( FsaDfaState *dfaState) { return m_DfaRepos->m_DistribRepos.ToVar( dfaState->DistribId()); }

    FsaCrate::Var           DfaTransition( FsaDfaState  *dfaState, const DistribCrate::Var &dVar, uint8_t chrId) 
    {   
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

    DistribCrate::Var       FetchDistib( FsaDfaState *dfaState) { return m_Distribs.VarId( dfaState->DistribId()); }

    FsaCrate::Var           DfaTransition( FsaDfaState  *dfaState, const DistribCrate::Var &dVar, uint8_t chrId) 
    {   
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
    
    Sg_Parapet( void) 
    {}

    void        Load( const FsaCrate::Var &rootState ) { m_CurState = rootState;  }
    
    bool        IsLoaded( void) const  { return !!m_CurState; }

template < typename Atelier>
    bool        Advance( Atelier *dfaAtelier, uint8_t chrId)
    { 
        switch ( m_CurState.GetType())
        { 
            case FsaCrate::template TypeOf< FsaDfaState>() : 
            {
                FsaDfaState             *dfaState = static_cast< FsaDfaState *>( m_CurState.GetEntry());;
                m_CurState =  dfaAtelier->DfaTransition( dfaState, dfaAtelier->FetchDistib( dfaState), chrId); 
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaUniXState>() : m_CurState =  dfaAtelier->DfaUniXTransition( m_CurState.GetEntry(), chrId); break;
            default : m_CurState = FsaCrate::Var(); break;
        }  
        //CV_PREFETCH_CACHE( m_CurState.GetEntry())
        return !!m_CurState;
    }  

    bool    HasTokens( void)
    { 
        return ( m_CurState.GetType() == FsaCrate::template TypeOf< FsaDfaState>()) && static_cast< FsaDfaState*>( m_CurState.GetEntry())->SzTokens();  
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
    FsaCrate::Var                               m_Root;
    uint64_t                                    m_Curr; 
    std::array< Sg_Parapet, 64>                 m_Parapets; 
    std::array< uint64_t, 64>                   m_Starts; 
    uint64_t                                    m_Allocbits;
    TokenGram                                   *m_TokenSet;   
    
    Sg_Bulwark( void)
        : m_DfaAtelier( NULL), m_Curr( 0), m_TokenSet( NULL), m_Allocbits( 0) 
    {} 

    void    Setup( Atelier *dfaAtelier)
    {
        m_DfaAtelier = dfaAtelier;
        m_Root = dfaAtelier->RootState();
        LoadRootAt( 0);
        m_Starts.fill( 0);
    }
        
    void            DumpTokens( Sg_Parapet  *parapet, uint64_t start)
    { 
        Cv_CArr< uint64_t>      tokens = parapet->Tokens(); 
        for ( uint32_t i = 0; i < tokens.Size(); ++i)
            m_TokenSet->Append( Sg_MatchData( start, uint32_t( m_Curr -start), tokens[ i]));
    }
    
    bool    LoadRootAt( uint32_t pickInd)
    { 
        Sg_Parapet      *curent = &m_Parapets[ pickInd];  
        curent->Load( m_Root); 
        m_Starts[ pickInd] = m_Curr;
        m_Allocbits = ( m_Allocbits | ( uint64_t( 1) << pickInd));  
        //CV_PREFETCH_CACHE( m_Root.GetEntry())
        return true;
    }

    bool    Play( uint8_t chrId)
    {    
        uint32_t    pickInd = CV_UINT32_MAX;
        uint64_t    allocbits = 0;
        for ( uint32_t ind = 0; m_Allocbits; ind++, m_Allocbits >>= 1)  
            if ( m_Allocbits & 1)
            {
                Sg_Parapet      *parapet = &m_Parapets[ ind];
                if ( !parapet->Advance( m_DfaAtelier, chrId)) 
                    pickInd = ind;   
                else
                {
                    allocbits = ( uint64_t( 1) << ind) | allocbits;
                    if ( m_TokenSet && parapet->HasTokens())
                        DumpTokens( parapet, m_Starts[ ind]); 
                }
            } 
        m_Allocbits = allocbits;
        ++m_Curr; 
        if ( pickInd != CV_UINT32_MAX)
            return LoadRootAt( pickInd);
    
        uint64_t    freeBits = ~allocbits;
        for ( uint32_t j = 0; freeBits; j++, freeBits >>= 1)  
            if ( freeBits & 1) 
                return LoadRootAt( j);   
        return false; 
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
