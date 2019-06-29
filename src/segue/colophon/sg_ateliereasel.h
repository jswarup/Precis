// sg_ateliereasel.h ___________________________________________________________________________________________________________________
#pragma once

#include    "segue/colophon/sg_atelier.h" 
 
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

template < typename Vita, typename Atelier>
struct Sg_AtelierEasel : public Sg_WorkEasel< Sg_AtelierEasel< Vita, Atelier>, Vita, Cv_AtelierStats>
{
    typedef Sg_WorkEasel< Sg_AtelierEasel< Vita, Atelier>, Vita, Cv_AtelierStats>     Base;
    
    typedef typename Vita::Datagram             Datagram;
    typedef typename Vita::InPort               InPort;
    typedef typename InPort::Wharf              Wharf;
    typedef typename Base::Stats                Stats;
    typedef typename Vita::OutTokPort           OutTokPort;
    typedef typename OutTokPort::Wharf          OutTokWharf;
    typedef typename Vita::TokenGram            TokenGram;    
    
    InPort                                      m_InDataPort;
    OutTokPort                                  m_TokOutPort;
    Atelier                                     *m_Atelier; 
    Sg_Bastion< Atelier>                        m_Bastion;
    uint64_t                                    m_Bytes;
    uint32_t                                    m_AtelierEaseld;
    uint32_t                                    m_AtelierEaselnd;
    uint32_t                                    m_AtelierEaseSz; 
    bool                                        m_CloseFlg; 

    Sg_AtelierEasel( const std::string &name = "Atelier") 
        : Base( name), m_Bytes( 0), m_AtelierEaseld( 0), m_AtelierEaselnd( 0), m_AtelierEaseSz( 1), m_CloseFlg( false)
    {}


    //_____________________________________________________________________________________________________________________________

    void SetEasel( uint32_t ind, uint32_t sz)
    {
        m_AtelierEaseld = m_AtelierEaselnd = ind;
        m_AtelierEaseSz = sz;
    }

    //_____________________________________________________________________________________________________________________________

    bool    DoInit( Vita *vita)
    {
        if ( !Base::DoInit( vita))
            return false;  

        if ( vita->m_ImgFile.size())  
            m_Bastion.Setup( vita->m_Atelier);  
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
        Wharf           wharf( &m_InDataPort);
        uint32_t        szBurst = wharf.Size(); 
        OutTokWharf     tokWharf( &m_TokOutPort);

        uint32_t        szTokBurst = tokWharf.Size(); 
        szTokBurst = tokWharf.ProbeSzFree( szTokBurst);

        if ( !szBurst && wharf.IsPrevClose() && (( m_CloseFlg = true)) && wharf.SetClose()  && tokWharf.SetClose())
            return;
 
        if ( !szBurst || !szTokBurst)
        {
            stats->m_ChokeSz.Incr(); 
            wharf.SetSize( 0);
            tokWharf.SetSize( 0);
            return;
        }  
        uint32_t    dInd = 0;
        uint32_t    tokInd = 0; 
        m_Bastion.FixOrigin( m_Bytes);
        TokenGram  *tokenSet =  tokWharf.AllocFree(); 
        tokenSet->SetOrigin( m_Bytes);
        for ( ; dInd < szBurst;  dInd++)
        {
            bool        rootScanFlg = (m_AtelierEaselnd++ % m_AtelierEaseSz ) == 0;
            Datagram    *datagram = wharf.Get( dInd);  
            Cv_Seq      dataSeq( datagram->PtrAt( 0), datagram->SzFill());
            m_Bytes += dataSeq.Size();
            rootScanFlg ? m_Bastion.Play( dataSeq, tokenSet) :  m_Bastion.PlayScan( dataSeq, tokenSet);
            if ( wharf.IsTail()) 
                wharf.Discard( datagram);
        } 
        stats->m_Matches += tokenSet->SzFill();
        if ( tokWharf.IsTail()) 
            tokWharf.Discard( tokenSet);
        else
            tokWharf.Set( tokInd++, tokenSet);
        wharf.SetSize( dInd);
        tokWharf.SetSize( tokInd); 
        return;
    }
}; 
 
//_____________________________________________________________________________________________________________________________
