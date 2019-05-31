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
    
    uint8_t             ByteCode( uint8_t chr )   { return m_DfaRepos->m_DistribRepos.m_Base.Image( chr); }
    FsaDfaState         *RootState( void) {  return static_cast< FsaDfaState *>( m_DfaRepos->ToVar( m_DfaRepos->m_RootId).GetEntry()); }
    DistribCrate::Var   DistribVar( const DistribRepos::Id &dId) {  return  m_DfaRepos->m_DistribRepos.ToVar( dId); }
    FsaDfaState         *Transition( FsaDfaState *state, uint8_t byteCode) { return static_cast< FsaDfaState *>( m_DfaRepos->ToVar( state->Dests().At( byteCode)).GetEntry()); }
    
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

    uint8_t             ByteCode( uint8_t chr )  { return m_Distribs.Base()->Image( chr); }
    FsaDfaState         *RootState( void)  {  return static_cast< FsaDfaState *>( m_States.ToVar( m_DfaBlossom.RootId()).GetEntry()); }
    DistribCrate::Var   DistribVar( const DistribRepos::Id &dId) {  return  m_Distribs.ToVar( dId); }
    FsaDfaState         *Transition( FsaDfaState *state, uint8_t byteCode)  { return static_cast< FsaDfaState *>( m_States.ToVar( state->Dests().At( byteCode)).GetEntry()); }

};


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

template < typename Atelier>
    bool        Advance( Atelier *dfaAtelier, uint8_t chrId)
    {
        DistribCrate::Var   dVar = dfaAtelier->DistribVar( m_CurState->DistribId());
        uint8_t             img = dVar( [ chrId]( auto k) { return k->Image( chrId); }); 
        m_CurState = dfaAtelier->Transition( m_CurState, img);
        return !!m_CurState;
    }

    Cv_CArr< uint64_t>      Tokens( void) { return m_CurState->Tokens(); }
};

//_____________________________________________________________________________________________________________________________

struct Sg_Rampart
{ 
    Sg_DfaReposAtelier  m_DfaAtelier;
    Sg_Parapet          m_Parapet;   
    uint64_t            m_Curr;     

    Sg_Rampart( void)
        : m_Curr( 0)
    {}

    void        SetDfaRepos( FsaDfaRepos *dfaRepos) { m_DfaAtelier.m_DfaRepos = dfaRepos; }

    bool        Play( uint8_t chr)
    {
        uint8_t     chrId = m_DfaAtelier.ByteCode( chr);
        if ( !m_Parapet.IsLoaded())
        {        
            FsaDfaState     *rootDfaState = static_cast< FsaDfaState *>( m_DfaAtelier.RootState());
            m_Parapet.Load( rootDfaState, m_Curr);
        }
        ++m_Curr;
        if ( !m_Parapet.Advance( &m_DfaAtelier, chrId))
            return false;
        Cv_CArr< uint64_t>      tokens = m_Parapet.Tokens();

        for ( uint32_t i = 0; i < tokens.Size(); ++i)
            std::cout << m_Parapet.Start() << " " << ( m_Curr -m_Parapet.Start()) << " " <<  tokens[ i] << "\n";
        return true; 
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

    void    Dump( std::ostream &strm)
    {
        strm << m_Start << " " << m_Len << " " <<  m_Token << "\n";
    }
};

//_____________________________________________________________________________________________________________________________

template < uint32_t Sz, uint32_t TokSz>
struct Sg_Bulwark
{ 
     
    uint64_t                                    m_Curr;    
    std::array< Sg_Parapet, Sz>                 m_Parapets; 
    Sg_Bitset< Sz>                              m_Allocbits;
    Cv_Array< Sg_MatchData, TokSz>              m_TokenSet; 

    Sg_Bulwark( void)
        : m_Curr( 0) 
    { 
    }
     
    void            DumpTokens( Sg_Parapet  *parapet)
    { 
        Cv_CArr< uint64_t>      tokens = parapet->Tokens(); 
        for ( uint32_t i = 0; i < tokens.Size(); ++i)
            m_TokenSet.Append( Sg_MatchData( parapet->Start(), uint32_t( m_Curr -parapet->Start()), tokens[ i]));
        return;
    }

template < typename Atelier>
    bool    Play( Atelier *dfaAtelier, uint8_t chr)
    { 
        uint8_t                 chrId = dfaAtelier->ByteCode( chr);  
        Sg_Bitset< Sz>          allocbits;  
        m_Allocbits.ForAllTrue( [this, dfaAtelier]( uint32_t ind, uint8_t chrId, Sg_Bitset< Sz> *allocbits)
            {
                Sg_Parapet      *parapet = &m_Parapets[ ind];
                if ( !parapet->Advance( dfaAtelier, chrId))
                    return;
                allocbits->Set( ind, true); 
                DumpTokens( &m_Parapets[ ind]);
                return;
            }, chrId, &allocbits);  
        
        uint32_t        pickInd =  allocbits.Index( false) ;
        if ( pickInd != CV_UINT32_MAX)
        {
            return false;
        }

        Sg_Parapet      *curent = &m_Parapets[ pickInd];  
        curent->Load( dfaAtelier->RootState(), m_Curr); 
        if ( curent->Advance( dfaAtelier, chrId))
            allocbits.Set( pickInd, true);    
        ++m_Curr;
        m_Allocbits = allocbits;
        return true; 
    }    
};

/*
//_____________________________________________________________________________________________________________________________

template < typename Wharf>
struct Sg_Bastion
{
    enum   {
        CSz = Wharf::Dock::DataCarousal::CarousalSz,
        BSz = 256,
    };

    struct  DataTrack
    {
        uint16_t    m_UseCount;
        uint16_t    m_ScanStart;

        DataTrack( void)
            : m_UseCount( 0), m_ScanStart( 0)
        {}
    };
    struct  Bulwark : public Sg_Parapet
    {
        uint16_t        m_TrackIndex;
        uint16_t        m_TrackStart;

        Bulwark( void)  
            : m_TrackIndex( 0), m_TrackStart( 0)
        {}
        
        void    Load( FsaDfaState *rootDfaState, uint64_t curr, uint16_t trackIndex, uint16_t trackStart)
        {
            m_TrackIndex = trackIndex;
            m_TrackStart = trackStart;
            Sg_Parapet::Load( rootDfaState, curr);
        }
 
        bool    Advance( FsaDfaRepos *dfaRepos, Wharf *wharf)
        {
            DistribCrate::Var   dVar = dfaRepos->m_DistribRepos.ToVar( m_CurState->DistribId());
            uint8_t             img = dVar( [ chr]( auto k) { return k->Image( chr); }); 
            m_CurState = static_cast< FsaDfaState *>( dfaRepos->ToVar( m_CurState->Dests().At( img)).GetEntry());
            return !!m_CurState;
                    }
    };

    DataTrack                   m_Track[ CSz];
    Cv_Array< Bulwark, BSz>     m_Bulwarks;
    FsaDfaRepos                 *m_DfaRepos; 
    uint32_t                    m_LastDGInd;    // just for datagram conversion
    uint16_t                    m_TrackIndex;
    uint16_t                    m_TrackStart;
    uint64_t                    m_Curr;
    
    Sg_Bastion( void)
        : m_DfaRepos( NULL), m_LastDGInd( 0), m_TrackIndex( 0), m_TrackStart( 0), m_Curr( 0)
    {}

    void        SetDfaRepos( FsaDfaRepos *dfaRepos) { m_DfaRepos = dfaRepos; }


    uint32_t    Play( Wharf *wharf)
    {
        FsaDfaState     *rootDfaState = static_cast< FsaDfaState *>( m_DfaRepos->ToVar( m_DfaRepos->m_RootId).GetEntry());
        uint32_t        szBurst = wharf->Size(); 
        Sg_Partition    *m_Base = &m_DfaRepos->m_DistribRepos.m_Base;
        for ( uint32_t dInd = m_LastDGInd -wharf->Begin(); dInd < szBurst;  dInd++, m_LastDGInd++)
        {
            auto    *datagram = wharf->Get( dInd); 
            for ( uint32_t k = 0; k < datagram->SzFill(); ++k)
            {
                uint8_t     &chr = datagram->At( k);
                chr = m_Base->Image( chr);
            }
        }
        Cv_Array< Bulwark, BSz>     bulwarks;
        for ( uint32_t i = 0; i < m_Bulwarks.SzFill(); ++i)
        {
            Bulwark     *bulwark = m_Bulwarks.PtrAt( i);
            bulwark->Advance( m_DfaRepos, wharf);
        }

        uint32_t        dInd = 0;
        for ( ; dInd < szBurst;  dInd++)
        {
            auto    *datagram = wharf->Get( dInd); 
            for ( uint32_t k = 0; k < datagram->SzFill(); ++k)
            {
                uint8_t     chr = datagram->At( k);
                //m_Ramparts.Play( chr);
            }
        }
        return dInd;
    }
};
*/
//_____________________________________________________________________________________________________________________________
