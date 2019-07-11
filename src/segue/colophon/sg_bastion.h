// sg_bastion.h ___________________________________________________________________________________________________________________
#pragma once

//_____________________________________________________________________________________________________________________________

struct Sg_MatchData
{
    uint32_t                m_Start;
    uint32_t                m_Len;
    uint64_t                m_Token;

    Sg_MatchData( void) 
    {}

    Sg_MatchData( uint32_t start, uint32_t len, uint64_t token)
        : m_Start( start), m_Len( len), m_Token( token)
    {}

    friend std::ostream &operator<<( std::ostream &strm, const Sg_MatchData &md)
    {
        strm << md.m_Start << "," << md.m_Len << "," <<  md.m_Token << "\n";
        return strm;
    }

template < uint32_t Sz>
    void                Dump( Cv_SpritzBuf< Sz> &ostrm, uint64_t origin)
    {
        ostrm << ( origin + m_Start) << ',' << m_Len << ',' <<  m_Token << '\n'; 
    }
};

//_____________________________________________________________________________________________________________________________

struct Sg_Tokengram 
{
    uint64_t                            m_Origin;
    Cv_Array< Sg_MatchData, 4096>       m_Tokens;

    Sg_Tokengram( void)
        :  m_Origin( 0)
    {}

    void    SetOrigin( uint64_t origin)  {   m_Origin = origin; }

    uint32_t            SzFill( void) const { return m_Tokens.SzFill(); }   
    uint32_t            SzVoid( void) const { return m_Tokens.SzVoid(); }     
    const Sg_MatchData  &At( uint32_t k) const { return m_Tokens[ k]; } 
    void                Append( const Sg_MatchData &x) { m_Tokens.Append( x); }   
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
    uint32_t                DumpTokens( TokenGram  *tokenSet,  uint32_t start, uint32_t end)
    {
        Cv_Seq< uint64_t>      tokens = Tokens();
        if ( tokenSet->SzVoid() < tokens.Size())
            return tokens.Size();

        for ( uint32_t i = 0; i < tokens.Size(); ++i)
            tokenSet->Append( Sg_MatchData( start, uint32_t( end -start), tokens[ i]));
        return 0;
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
            uint8_t     chrId = m_DfaAtelier->ByteCode( chr);
            bool        proceed = Play( chrId);
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

    const FsaId     &State( uint32_t ind) const { return m_States[ ind]; }
    void            SetState( uint32_t ind, const FsaId &id) { m_States[ ind] = id; }


    uint32_t        FixOrigin( uint64_t origin) 
    { 
        uint32_t    shift = uint32_t( origin -m_Origin);
        m_Origin += shift; 
        for ( uint32_t i = 0; i < m_AllocSz; ++i)
            m_Starts[ i] -= shift;
        return shift;
    }

    void        SetStart( uint32_t ind, uint32_t val) { m_Starts[ ind] = val; }


template < typename Atelier, typename TokenGram>
    uint32_t        ScanCycle( Atelier *atelier, TokenGram  *tokenSet, uint32_t curr, const Cv_Seq< uint8_t> &chrs, uint32_t sz)
    {
        std::array< uint8_t, Sz>        allocInds;
        uint16_t                        szAlloc = 0;
        uint32_t                        szDroppedToken = 0;
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
                    szDroppedToken += parapet.DumpTokens( tokenSet, m_Starts[ ind], curr +k +1);
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
        return szDroppedToken;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename Atelier>
struct Sg_Bastion
{
    Sg_Bulwark              m_BulWark;
    Atelier                 *m_DfaAtelier;   

    Sg_Bastion( void)
        : m_DfaAtelier( NULL) 
    {}

    void    Setup( Atelier *dfaAtelier)
    {
        m_DfaAtelier = dfaAtelier; 
    }

    void    FixOrigin( uint64_t origin) 
    {  
        m_BulWark.FixOrigin( origin);  
    } 

template <  typename TokenGram>
    auto            ScanRoot(  TokenGram  *tokenSet, uint32_t rootInd, const Cv_Seq< uint8_t> &chrs, uint32_t curr, uint32_t *pLen, uint32_t *pSzDroppedToken)
    { 
        uint32_t            i = 0; 
        FsaDfaRepos::Id     nxStateId;
        for ( ;  !nxStateId.IsValid() && i < chrs.Size(); ++i)
            nxStateId = m_DfaAtelier->AdvanceRoot( chrs[ i]); 

        *pLen = i;
        if ( !nxStateId.IsValid())
            return false;
        Sg_Parapet      nxParapet = m_DfaAtelier->VarFromId( nxStateId); 
        m_BulWark.SetState( rootInd, nxStateId); 
        m_BulWark.SetStart( rootInd, curr +i -1);

        if ( tokenSet && nxParapet.HasTokens())
            *pSzDroppedToken += nxParapet.DumpTokens( tokenSet, curr +i -1, curr +i);
        return true;
    }

    // return true if context is persists
template < typename TokenGram>
    uint32_t    PlayScan( Cv_Seq< uint8_t> chrs, uint32_t *pCurr, TokenGram  *tokenSet)
    { 
        uint32_t    szDroppedToken = 0;
        if ( m_BulWark.SzAlloc())                                                           // scan for root-match in the buffer.
            szDroppedToken = m_BulWark.ScanCycle( m_DfaAtelier, tokenSet, *pCurr, chrs, chrs.Size());        // scan-sycle the rest upto the scan-Marker 
        *pCurr += chrs.Size();
        return szDroppedToken;
    }

    // return true if context is persists
template < typename TokenGram>
    uint32_t    Play( Cv_Seq< uint8_t> chrs, uint32_t *pCurr, TokenGram  *tokenSet)
    { 
        uint32_t    szDroppedToken = 0;
        uint32_t    szAlloc = m_BulWark.SzAlloc();
        while ( chrs.Size())
        {   
            uint32_t    rootInd = m_BulWark.FetchFree();
            uint32_t    szScan = chrs.Size(); 
            bool        injectFlg = ScanRoot(  tokenSet, rootInd, chrs, *pCurr, &szScan, &szDroppedToken);   
            if ( szAlloc)                                                                       // scan for root-match in the buffer.
                szDroppedToken += m_BulWark.ScanCycle( m_DfaAtelier, tokenSet, *pCurr, chrs, szScan);                       // scan-sycle the rest upto the scan-Marker
            *pCurr += szScan;
            chrs.Advance( szScan); 
            injectFlg  ? m_BulWark.MarkOccupied( rootInd) : m_BulWark.MarkFree( rootInd);
            szAlloc = m_BulWark.SzAlloc();
        }
        return szDroppedToken;
    }
};
