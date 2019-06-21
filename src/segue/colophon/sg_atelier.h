// sg_atelier.h ___________________________________________________________________________________________________________________
#pragma once


#include    "segue/epigraph/sg_bitset.h"
#include    "segue/epigraph/sg_partition.h"
#include    "segue/colophon/sg_dfastate.h"

//_____________________________________________________________________________________________________________________________

using namespace Sg_RExp;

//_____________________________________________________________________________________________________________________________

struct Sg_DfaReposAtelier
{
    FsaDfaRepos     *m_DfaRepos;

    Sg_DfaReposAtelier( FsaDfaRepos *dfaRepos = NULL)
        : m_DfaRepos( dfaRepos)
    {}

    FsaCrate::Var           VarFromId( const FsaDfaRepos::Id &id) const { return m_DfaRepos->ToVar( id ); }
    
    uint8_t                 ByteCode( uint8_t chr ) const  { return m_DfaRepos->m_DistribRepos.m_Base.Image( chr); }

    FsaCrate::Var           RootState( void) {  return VarFromId( m_DfaRepos->m_RootId); }


    DistribCrate::Var       FetchDistib( FsaDfaState *dfaState) { return m_DfaRepos->m_DistribRepos.ToVar( dfaState->DistribId()); }

    FsaDfaRepos::Id         DfaTransition( FsaDfaState  *dfaState, const DistribCrate::Var &dVar, uint8_t chrId)
    {
        uint8_t             img = dVar( [ chrId]( auto k) { return k->Image( chrId); });
        return dfaState->DestAt( img);
    }

    FsaDfaRepos::Id          DfaUniXTransition( FsaState *state, uint8_t chrId)
    {
        FsaDfaUniXState     *dfaState = static_cast< FsaDfaUniXState *>( state);
        if ( chrId == dfaState->m_Byte)
            return  dfaState->m_Dest;
        return FsaDfaRepos::Id();
    }

    FsaDfaRepos::Id         Advance( const FsaCrate::Var &state, uint8_t chrId)
    {
        FsaDfaRepos::Id   nxState;
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

    FsaCrate::Var           VarFromId( const FsaDfaRepos::Id &id) const { return m_States.VarId( id ); }

    uint8_t                 ByteCode( uint8_t chr ) const  { return m_Distribs.Base()->Image( chr); }

    FsaCrate::Var           RootState( void)  {  return m_States.ToVar( m_DfaBlossom.RootId()); }

    DistribCrate::Var       FetchDistib( FsaDfaState *dfaState) { return m_Distribs.VarId( dfaState->DistribId()); }

    FsaDfaRepos::Id         DfaTransition( FsaDfaState  *dfaState, const DistribCrate::Var &dVar, uint8_t chrId)
    {
        return  dfaState->DestAt( dVar( [ chrId]( auto k) { return k->Image( chrId); }));
    }

    FsaDfaRepos::Id         DfaUniXTransition( FsaDfaUniXState  *dfaState, uint8_t chrId)
    {
        if ( chrId == dfaState->m_Byte)
            return  dfaState->m_Dest;
        return FsaDfaRepos::Id();
    }


    FsaDfaRepos::Id         Advance( const FsaCrate::Var &state, uint8_t chrId)
    {
        FsaDfaRepos::Id   nxState;
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

    std::array< FsaDfaRepos::Id, 8>   Advance( const FsaCrate::Var &state, uint64_t chrOct)
    {
        uint8_t                         *chrIds = ( uint8_t *) &chrOct;
        std::array< FsaDfaRepos::Id, 8>   nxStates;
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

    Sg_Parapet( const FsaCrate::Var &state)
        : m_CurState( state)
    {}

    void        SetState( const FsaCrate::Var &rootState ) { m_CurState = rootState;  } 

template < typename Atelier>
    bool        Advance( Atelier *dfaAtelier, uint8_t chrId)
    {
        m_CurState = dfaAtelier->VarFromId( dfaAtelier->Advance( m_CurState, chrId));
        return !!m_CurState;
    }

    bool    HasTokens( void)
    {
        return ( m_CurState.GetType() == FsaCrate::template TypeOf< FsaDfaState>()) && static_cast< FsaDfaState*>( m_CurState.GetEntry())->SzTokens();
    }

    Cv_Seq< uint64_t>      Tokens( void) {  return m_CurState.Tokens(); }

template < typename TokenGram>
    void            DumpTokens( TokenGram  *tokenSet, uint64_t start, uint64_t end)
    {
        Cv_Seq< uint64_t>      tokens = Tokens();
        if ( tokenSet->SzVoid() < tokens.Size())
                return;
        for ( uint32_t i = 0; i < tokens.Size(); ++i)
            tokenSet->Append( Sg_MatchData( start, uint32_t( end -start), tokens[ i]));
        return;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename Atelier, typename TokenGram>
struct Sg_Citadel
{

    Atelier                                     *m_DfaAtelier;
    FsaCrate::Var                               m_Root;
    uint32_t                                    m_Curr;
    std::array< Sg_Parapet, 64>                 m_Parapets;
    std::array< uint32_t, 64>                   m_Starts;
    uint64_t                                    m_Allocbits;
    TokenGram                                   *m_TokenSet;

    Sg_Citadel( void)
        : m_DfaAtelier( NULL), m_Curr( 0), m_TokenSet( NULL), m_Allocbits( 0)
    {}

    void    Setup( Atelier *dfaAtelier)
    {
        m_DfaAtelier = dfaAtelier;
        m_Root =  dfaAtelier->RootState();
        LoadRootAt( 0);
        m_Starts.fill( 0);
    }

    bool    LoadRootAt( uint32_t pickInd)
    {
        Sg_Parapet      *curent = &m_Parapets[ pickInd];
        curent->SetState( m_Root);
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
                        parapet->DumpTokens( m_TokenSet, m_Starts[ ind], m_Curr +1);
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

class  Sg_Bulwark
{
    enum {
        Sz = 64
    };
    
    typedef FsaDfaRepos::Id             FsaId;

    uint64_t                            m_Origin;
    std::array< FsaId, Sz>              m_States;
    std::array< uint32_t, Sz>           m_Starts;
    std::array< uint8_t, Sz>            m_FreeInds;
    std::array< uint8_t, Sz>            m_AllocInds;
    std::array< uint8_t, Sz>            m_FinInds;
    uint16_t                            m_FreeSz;
    uint16_t                            m_AllocSz;
    uint16_t                            m_TokSz;

public: 
    Sg_Bulwark( void)
        :  m_FreeSz( Sz), m_AllocSz( 0), m_Origin( 0)
    {
        uint32_t sz = sizeof( Sg_Bulwark);
        m_Starts.fill( 0);
        Cv_For< Sz>::RunAll( [this]( uint32_t ind) { m_FreeInds[ ind] = Sz -1 -ind; });
    }

    uint16_t    SzAlloc( void) const { return m_AllocSz; }
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

    const FsaId &State( uint32_t ind) const { return m_States[ ind]; }
    void        SetState( uint32_t ind, const FsaId &id) { m_States[ ind] = id; }
    

    uint64_t    StartFromOrigin( uint32_t ind) const { return m_Origin + m_Starts[ ind]; }

    void        SetOrigin( uint64_t origin) { m_Origin = origin; }
    void        SetStart( uint32_t ind, uint32_t val) { m_Starts[ ind] = val; }

template < typename Atelier, typename TokenGram>
    auto            ScanRoot( Atelier *atelier, TokenGram  *tokenSet, const FsaCrate::Var &root, uint32_t curr, uint32_t rootInd, const Cv_Seq< uint8_t> &chrs)
    { 
        uint32_t            i = 0;
        FsaDfaRepos::Id     nxStateId;
        for ( ;  !nxStateId.IsValid() && i < chrs.Size(); ++i)
            nxStateId = atelier->Advance( root, chrs[ i]); 

        if ( ! nxStateId.IsValid())
            return std::make_tuple( false, chrs.Size());
        Sg_Parapet      nxParapet = atelier->VarFromId( nxStateId); 
        SetState( rootInd, nxStateId); 
        SetStart( rootInd, curr +i -1);

        if ( tokenSet && nxParapet.HasTokens())
            nxParapet.DumpTokens( tokenSet, curr +i -1, curr +i);
        return std::make_tuple( true, i);
    }

template < typename Atelier, typename TokenGram>
    void        ScanCycle( Atelier *atelier, TokenGram  *tokenSet, uint64_t curr, const Cv_Seq< uint8_t> &chrs, uint32_t sz)
    {
        std::array< uint8_t, Sz>        allocInds;
        uint16_t                        szAlloc = 0;
        for ( uint16_t  q = 0; q < m_AllocSz; ++q)
        {
            uint32_t            ind = m_AllocInds[ q];
            Sg_Parapet          parapet = atelier->VarFromId( State( ind)); 
            FsaId               nxStateId;
            for ( uint32_t k = 0; k < sz; ++k)
            {
                nxStateId = atelier->Advance( parapet.m_CurState, chrs[ k]);
                if ( !nxStateId.IsValid()) 
                    break; 

                parapet.SetState( atelier->VarFromId( nxStateId)); 
                if ( tokenSet && parapet.HasTokens())
                    parapet.DumpTokens( tokenSet, StartFromOrigin( ind), curr +k +1);
            }
            if ( nxStateId.IsValid())
            {
                allocInds[ szAlloc++] = ( uint8_t) ind;  
                SetState( ind, nxStateId);
            }
            else
                MarkFree( ind);
        }
        std::copy( &allocInds[ 0],  &allocInds[ szAlloc],  &m_AllocInds[ 0]);
        m_AllocSz = szAlloc;
        return;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename Atelier, typename TokenGram>
struct Sg_Bastion
{
    Sg_Bulwark              *m_BulWark;
    Atelier                 *m_DfaAtelier;
    TokenGram               *m_TokenSet;
    uint32_t                m_Curr;
    FsaCrate::Var           m_Root; 

    Sg_Bastion( void)
        : m_DfaAtelier( NULL), m_TokenSet( NULL), m_Curr( 0) 
    {}

    void    Setup( Atelier *dfaAtelier)
    {
        m_DfaAtelier = dfaAtelier;
        m_Root = dfaAtelier->RootState();
    }

    void    SetBulwark( Sg_Bulwark *bulWark, uint64_t origin) 
    { 
        m_BulWark = bulWark; 
        bulWark->SetOrigin(  origin);
        m_Curr = 0;
    } 

    // return true if context is persists
    bool    PlayScan( Cv_Seq< uint8_t> chrs)
    { 
        if ( m_BulWark->SzAlloc())                                                                                               // scan for root-match in the buffer.
            m_BulWark->ScanCycle( m_DfaAtelier, m_TokenSet, m_Curr, chrs, chrs.Size());                   // scan-sycle the rest upto the scan-Marker 
        return !!m_BulWark->SzAlloc();
    }

    // return true if context is persists
    bool    Play( Cv_Seq< uint8_t> chrs)
    { 
        uint32_t    szAlloc = m_BulWark->SzAlloc();
        while ( chrs.Size())
        {   
            uint32_t    rootInd = m_BulWark->FetchFree();
            uint32_t    szScan = chrs.Size();
            bool        injectFlg = false; 

            std::tie( injectFlg, szScan) = m_BulWark->ScanRoot( m_DfaAtelier, m_TokenSet, m_Root, m_Curr, rootInd, chrs);   

            if ( szAlloc)                                                                       // scan for root-match in the buffer.
                m_BulWark->ScanCycle( m_DfaAtelier, m_TokenSet, m_Curr, chrs, szScan);                       // scan-sycle the rest upto the scan-Marker
            m_Curr += szScan;
            chrs.Advance( szScan); 
            injectFlg  ? m_BulWark->MarkOccupied( rootInd) : m_BulWark->MarkFree( rootInd);
            szAlloc = m_BulWark->SzAlloc();
        }
        return !!szAlloc;
    }
};


//_____________________________________________________________________________________________________________________________
