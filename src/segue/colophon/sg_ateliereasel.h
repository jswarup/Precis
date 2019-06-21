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
    
    typedef Sg_Bastion< Atelier, TokenGram>     Bastion;

    InPort                                      m_InDataPort;
    OutTokPort                                  m_TokOutPort;
    Atelier                                     *m_Atelier; 
    uint64_t                                    m_Bytes;
    uint32_t                                    m_AtelierEaseld;
    uint32_t                                    m_AtelierEaseSz;
    Sg_Bulwark                                  m_BulWark;
    Bastion                                     m_Bastion;
    bool                                        m_CloseFlg;
    bool                                        m_SavedCtxtFlag;

    Sg_AtelierEasel( const std::string &name = "Atelier") 
        : Base( name), m_Bytes( 0), m_AtelierEaseld( 0), m_AtelierEaseSz( 1), m_CloseFlg( false), m_SavedCtxtFlag( false)
    {}

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
        
        //if ( szBurst > szTokBurst)
        //    szBurst = szTokBurst;
        if ( !szBurst || !szTokBurst)
        {
            stats->m_ChokeSz.Incr(); 
            wharf.SetSize( 0);
            tokWharf.SetSize( 0);
            return;
        }  
        uint32_t    dInd = 0;
        uint32_t    tokInd = 0; 
        m_Bastion.SetBulwark( &m_BulWark, m_Bytes);
        m_Bastion.m_TokenSet =  tokWharf.AllocFree();
        m_Bytes += szBurst;
        for ( ; dInd < szBurst;  dInd++)
        {
            bool        rootScanFlg = (m_AtelierEaseld++ % m_AtelierEaseSz ) == 0;
            Datagram    *datagram = wharf.Get( dInd);  
            Cv_Seq      dataSeq( datagram->m_ScanBuffer.PtrAt( 0), datagram->m_ScanBuffer.SzFill());
            m_SavedCtxtFlag = rootScanFlg ? m_Bastion.Play( dataSeq) : (  m_SavedCtxtFlag ? m_Bastion.PlayScan( dataSeq) : 0);
            if ( wharf.IsTail()) 
                wharf.Discard( datagram);
        } 
        stats->m_Matches += m_Bastion.m_TokenSet->SzFill();
        if ( tokWharf.IsTail()) 
            tokWharf.Discard( m_Bastion.m_TokenSet);
        else
            tokWharf.Set( tokInd++, m_Bastion.m_TokenSet);
        wharf.SetSize( dInd);
        tokWharf.SetSize( tokInd); 
        return;
    }
}; 

//_____________________________________________________________________________________________________________________________

template < typename Vita>
struct Sg_AtelierTokenEasel : public Sg_WorkEasel< Sg_AtelierTokenEasel< Vita>, Vita, Cv_AtelierStats>
{
    typedef Sg_WorkEasel< Sg_AtelierTokenEasel< Vita>, Vita, Cv_AtelierStats>     Base;

    typedef typename Vita::Datagram             Datagram;
    typedef typename Vita::InPort               InPort;
    typedef typename InPort::Wharf              Wharf;
    typedef typename Base::Stats                Stats; 
    typedef typename Vita::TokenGram            TokenGram; 

    Cv_File                                     m_OutFile;
    InPort                                      m_InDataPort;   

    Sg_AtelierTokenEasel( const std::string &name = "Token") 
        : Base( name) 
    {}

    //_____________________________________________________________________________________________________________________________

    bool    DoInit( Vita *vita)
    {
        if ( !Base::DoInit( vita))
            return false;  
        if ( !m_OutFile.Open( vita->m_TokenLogFile.c_str(), false))
            return false;
        return true;
    }

    bool    IsRunable( void)
    {
        return m_OutFile.IsActive();;
    }

    //_____________________________________________________________________________________________________________________________

    void    DoRunStep( void)
    {   
        Stats           *stats = this->CurStats();
        Wharf           wharf( &m_InDataPort);
        uint32_t        szBurst = wharf.Size();  

 
        if ( !szBurst)
        {
            stats->m_ChokeSz.Incr(); 
            wharf.SetSize( 0); 
            if ( wharf.IsPrevClose())
            { 
                wharf.SetClose();
                m_OutFile.Shut(); 
            }
            return;
        }  

        uint32_t    tokInd = 0;
        uint32_t    dInd = 0;
        uint32_t    tokCnt = 0;  
        std::stringstream   sstrm; 
        for ( ; dInd < szBurst;  dInd++)
        {
             
            Datagram    *datagram = wharf.Get( dInd);  
            TokenGram   *tokengram = &datagram->m_TokenData; 
            uint32_t    szWrite = tokengram->SzFill();   
            for ( uint32_t k = 0; k < szWrite; ++k)
                sstrm << tokengram->At( k);
            if ( wharf.IsTail()) 
                wharf.Discard( datagram);               
        } 
        m_OutFile.Write( sstrm.str().c_str(), uint32_t( sstrm.str().length()));
        wharf.SetSize( dInd); 
        stats->m_Matches += tokCnt; 
            
        return;
    }
}; 

//_____________________________________________________________________________________________________________________________
