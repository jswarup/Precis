// sg_atelier.h ___________________________________________________________________________________________________________________
#pragma once


#include    "segue/timbre/sg_bitset.h"
#include    "segue/timbre/sg_partition.h"
#include    "segue/tremolo/sg_dfastate.h"

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

    FsaCrate::Var           Advance( const FsaCrate::Var &state, uint8_t chrId)
    {
        FsaCrate::Var   nxState;
        switch ( state.GetType())
        {
            case FsaCrate::template TypeOf< FsaDfaState>() :
            {
                FsaDfaState             *dfaState = static_cast< FsaDfaState *>( state.GetEntry());
                nxState =   DfaTransition( dfaState, FetchDistib( dfaState), chrId);
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaUniXState>() :
            {
                nxState = DfaUniXTransition( state.GetEntry(), chrId);
                break;
            }
            default :
                break;
        }
        return nxState;
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
        return  m_States.VarId( dfaState->DestAt( dVar( [ chrId]( auto k) { return k->Image( chrId); })));
    }

    FsaCrate::Var           DfaUniXTransition( FsaDfaUniXState  *dfaState, uint8_t chrId)
    {
        if ( chrId == dfaState->m_Byte)
            return m_States.VarId(  dfaState->m_Dest);
        return FsaCrate::Var();
    }


    FsaCrate::Var           Advance( const FsaCrate::Var &state, uint8_t chrId)
    {
        FsaCrate::Var   nxState;
        switch ( state.GetType())
        {
            case FsaCrate::template TypeOf< FsaDfaState>() :
            {
                FsaDfaState             *dfaState = static_cast< FsaDfaState *>( state.GetEntry());
                nxState =   DfaTransition( dfaState, FetchDistib( dfaState), chrId);
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaUniXState>() :
            {
                FsaDfaUniXState     *dfaState = static_cast< FsaDfaUniXState *>( state.GetEntry());
                nxState = DfaUniXTransition( dfaState, chrId);
                break;
            }
            default :
                break;
        }
        return nxState;
    }

    std::array< FsaCrate::Var, 8>   Advance( const FsaCrate::Var &state, uint64_t chrOct)
    {
        uint8_t                         *chrIds = ( uint8_t *) &chrOct;
        std::array< FsaCrate::Var, 8>   nxStates;
        switch ( state.GetType())
        {
            case FsaCrate::template TypeOf< FsaDfaState>() :
            {
                FsaDfaState             *dfaState = static_cast< FsaDfaState *>( state.GetEntry());
                DistribCrate::Var       dVar = FetchDistib( dfaState);

                Cv_For< 8>::RunAll( [ this, dfaState, &dVar, chrIds, &nxStates]( uint32_t ind) {
                    nxStates[ ind] =   DfaTransition( dfaState, dVar, chrIds[ ind]);
                });
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaUniXState>() :
            {
                FsaDfaUniXState     *dfaState = static_cast< FsaDfaUniXState *>( state.GetEntry());
                Cv_For< 8>::RunAll( [ this, dfaState, chrIds, &nxStates]( uint32_t ind) {
                    nxStates[ ind] =   DfaUniXTransition(  dfaState, chrIds[ ind]);
                });
                break;
            }
            default :
                break;
        }
        return nxStates;
    }
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

struct Sg_Parapet
{
    FsaClip         m_CurState;

    Sg_Parapet( void)
    {}

    Sg_Parapet( const FsaClip &state)
        : m_CurState( state)
    {}

    void        Load( const FsaCrate::Var &rootState ) { m_CurState = rootState;  }

    bool        IsLoaded( void) const  { return !!m_CurState; }

template < typename Atelier>
    bool        Advance( Atelier *dfaAtelier, uint8_t chrId)
    {
        m_CurState = dfaAtelier->Advance( m_CurState, chrId);
        return !!m_CurState;
    }

    bool    HasTokens( void)
    {
        return ( m_CurState.GetType() == FsaCrate::template TypeOf< FsaDfaState>()) && static_cast< FsaDfaState*>( m_CurState.GetEntry())->SzTokens();
    }

    Cv_Seq< uint64_t>      Tokens( void) {  return m_CurState.Tokens(); }

template < typename TokenGram>
    void            DumpTokens( TokenGram  *tokenSet, uint64_t start, uint64_t curr)
    {
        Cv_Seq< uint64_t>      tokens = Tokens();
        for ( uint32_t i = 0; i < tokens.Size(); ++i)
            tokenSet->Append( Sg_MatchData( start, uint32_t( curr -start), tokens[ i]));
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
                        parapet->DumpTokens( m_TokenSet, m_Starts[ ind], m_Curr);
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

    bool    Play( const Cv_Seq< uint8_t> &chrs)
    {
        for ( uint32_t k = 0; k < chrs.Size(); ++k)
        {
            uint8_t     chr = chrs[ k];

            bool        proceed = Play( chr);
        }
        return true;
    }
};

//_____________________________________________________________________________________________________________________________

class  Sg_Rampart
{
    enum {
        Sz = 64
    };

    std::array< Sg_Parapet, Sz> m_Parapets;
    std::array< uint64_t, Sz>   m_Starts;
    std::array< uint8_t, Sz>    m_FreeInds;
    std::array< uint8_t, Sz>            m_AllocInds;
    uint16_t                            m_FreeSz;
    uint16_t                            m_AllocSz;

public: 
    Sg_Rampart( void)
        :  m_FreeSz( Sz), m_AllocSz( 0)
    {
        uint32_t sz = sizeof( Sg_Rampart);
        m_Starts.fill( 0);
        Cv_For< Sz>::RunAll( [this]( uint32_t ind) { m_FreeInds[ ind] = Sz -1 -ind; });
    }

    void        MarkOccupied( uint32_t ind) 
    { 
        m_AllocInds[ m_AllocSz++] = ( uint8_t) ind;  
    }

    uint32_t    FetchFree( void)   
    { 
        return m_FreeSz ?  m_FreeInds[ --m_FreeSz] : CV_UINT32_MAX; 
    }
    void        MarkFree( uint32_t ind)  
    {   
        m_FreeInds[ m_FreeSz++] = ( uint8_t) ind; 
    }

    Sg_Parapet  *Parapet( uint32_t ind) { return &m_Parapets[ ind]; }
    uint64_t    Start( uint32_t ind) const { return m_Starts[ ind]; }

    void        SetStart( uint32_t ind, uint64_t val) { m_Starts[ ind] = val; }


template < typename Atelier, typename TokenGram>
    void    ScanCycle( Atelier *atelier, TokenGram  *tokenSet, uint64_t curr, const Cv_Seq< uint8_t> &chrs, uint32_t sz)
    {
        std::array< uint8_t, Sz>        allocInds;
        uint16_t                        szAlloc = 0;
        for ( uint16_t i =  m_AllocSz; i > 0; --i)
        {
            uint32_t        ind = m_AllocInds[ i -1];
            Sg_Parapet      *parapet = &m_Parapets[ ind];
            bool            surviveFlg = true;
            for ( uint32_t k = 0; k < sz; ++k)
            {
                FsaCrate::Var       nxState = atelier->Advance( parapet->m_CurState, chrs[ k]);
                if ( !nxState)
                {
                    MarkFree( ind);
                    surviveFlg = false;
                    break;
                }
                parapet->m_CurState = nxState;
                if ( tokenSet && parapet->HasTokens())
                    parapet->DumpTokens( tokenSet, m_Starts[ ind] +k, curr);
            }
            if ( surviveFlg)
                allocInds[ szAlloc++] = ( uint8_t) ind;  
        }
        std::copy( &allocInds[ 0],  &allocInds[ szAlloc],  &m_AllocInds[ 0]);
        m_AllocSz = szAlloc;
        return;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename Atelier, typename TokenGram>
struct Sg_Bastion : public Sg_Rampart
{
    Atelier                 *m_DfaAtelier;
    uint64_t                m_Curr;
    FsaCrate::Var           m_Root;
    TokenGram               *m_TokenSet;

    Sg_Bastion( void)
        : m_DfaAtelier( NULL), m_Curr( 0), m_TokenSet( NULL)
    {}

    void    Setup( Atelier *dfaAtelier)
    {
        m_DfaAtelier = dfaAtelier;
        m_Root = dfaAtelier->RootState();
    }

    uint32_t    ScanRoot( uint32_t rootInd, const Cv_Seq< uint8_t> &chrs)
    {
        Sg_Parapet      *parapet = Parapet( rootInd);
        uint32_t        i = 0;
        for ( ; i < chrs.Size(); ++i)
        {
            FsaCrate::Var   nxState = m_DfaAtelier->Advance( m_Root, chrs[ i]);
            if ( ! nxState)
                continue;
            parapet->m_CurState = nxState;
            SetStart( rootInd, m_Curr);
            if ( m_TokenSet && parapet->HasTokens())
                parapet->DumpTokens( m_TokenSet, m_Curr, m_Curr +i);
            ++i;
            break;
        }
        return i;
    }

    bool    Play( Cv_Seq< uint8_t> chrs)
    {
        while ( chrs.Size())
        {
            uint32_t    rootInd = FetchFree();
            uint32_t    szScan = ScanRoot( rootInd, chrs);                                               // scan for root-match in the buffer.
            ScanCycle( m_DfaAtelier, m_TokenSet, m_Curr, chrs, szScan);   // scan-sycle the rest upto the scan-Marker
            m_Curr += szScan;
            chrs.Advance( szScan);
            if ( !chrs.Size())
            {
                MarkFree( rootInd);
                break;
            }
            MarkOccupied( rootInd);
        }
        return true;
    }
};


//_____________________________________________________________________________________________________________________________
