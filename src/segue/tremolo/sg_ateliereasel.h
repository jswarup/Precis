// sg_ateliereasel.h ___________________________________________________________________________________________________________________
#pragma once

#include    "segue/tremolo/sg_atelier.h" 
 
//_____________________________________________________________________________________________________________________________


struct  Cv_AtelierStats : public Cv_EaselStats
{
    typedef Cv_EaselStats       Base;

    Cv_Type< uint64_t>          m_Matches;

    uint64_t    Matches( Cv_AtelierStats *prev) { return m_Matches.Get() -prev->m_Matches.Get(); }

    void    LogStats( std::ostream &strm, Cv_AtelierStats *prev)
    {
        Base::LogStats( strm, prev);
        strm << "Matches[ " << Matches( prev) << "] ";
        return;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename Vita>
struct Sg_AtelierEasel : public Sg_WorkEasel< Sg_AtelierEasel< Vita>, Vita, Cv_AtelierStats>
{
    typedef Sg_WorkEasel< Sg_AtelierEasel< Vita>, Vita, Cv_AtelierStats>     Base;

    typedef typename Vita::Datagram     Datagram;
    typedef typename Vita::InPort       InPort;
    typedef typename InPort::Wharf      Wharf;
    typedef typename Base::Stats        Stats;
    typedef typename Vita::OutTokPort   OutTokPort;
    typedef typename OutTokPort::Wharf  OutTokWharf;
    typedef typename Vita::TokenGram    TokenGram;    

    InPort                              m_DataPort;
    OutTokPort                          m_TokOutPort;
    Sg_DfaReposAtelier                  *m_DfaReposAtelier;
    Sg_DfaBlossomAtelier                *m_DfaBlossomAtelier;
    FsaDfaRepos                         m_DfaRepos;
    Sg_Bulwark< 64, TokenGram>          m_Bulwark;
    bool                                m_CloseFlg;
    std::vector< uint8_t>               m_MemArr; 

    Sg_AtelierEasel( const std::string &name = "Atelier") 
        : Base( name), m_DfaReposAtelier( NULL), m_DfaBlossomAtelier( NULL), m_CloseFlg( false)
    {}

    //_____________________________________________________________________________________________________________________________

    bool    DoInit( Vita *vita)
    {
        if ( !Base::DoInit( vita))
            return false;  

        if ( vita->m_ImgFile.size()) 
        { 
            bool	                res = Cv_Aid::ReadVec( &m_MemArr, vita->m_ImgFile.c_str()); 
            if ( !res)
            {
                std::cerr << "Not Found : " << vita->m_ImgFile << '\n';
                return false;
            }
            m_DfaBlossomAtelier = new Sg_DfaBlossomAtelier(  &m_MemArr[ 0]); 
        }
        return true;
    }

    bool    IsRunable( void)
    {
        return !m_CloseFlg;
    }

    //_____________________________________________________________________________________________________________________________

    void    DoRunStep( void)
    {   
        Stats           *stats = this->CurStats();
        Wharf           wharf( &m_DataPort);
        uint32_t        szBurst = wharf.Size(); 
        OutTokWharf     tokWharf( &m_TokOutPort);

        uint32_t        szTokBurst = tokWharf.Size(); 
        szTokBurst = tokWharf.ProbeSzFree( szTokBurst);

        if ( !szBurst && wharf.IsClose() && (( m_CloseFlg = true)) && wharf.SetClose())
            return;
        
        if ( !szBurst || !szTokBurst)
        {
            stats->m_ChokeSz.Incr(); 
            wharf.SetSize( 0);
            tokWharf.SetSize( 0);
            return;
        } 
        uint32_t    dInd = 0;
        uint32_t    tokCnt = 0;
        for ( ; dInd < szBurst;  dInd++)
        {
            Datagram    *datagram = wharf.Get( dInd); 
            for ( uint32_t k = 0; k < datagram->SzFill(); ++k)
            {
                uint8_t     chr = datagram->At( k);
                bool        proceed = m_Bulwark.Play( m_DfaBlossomAtelier, chr);
            }
            if ( wharf.IsTail()) 
                wharf.Discard( datagram);
        }
        wharf.SetSize( dInd);
        stats->m_Matches += tokCnt;
        return;
    }
}; 

//_____________________________________________________________________________________________________________________________
