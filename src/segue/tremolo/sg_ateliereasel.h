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
    
    typedef Sg_Bulwark< Atelier, 64, TokenGram> Bulwark;

    InPort                              m_InDataPort;
    OutTokPort                          m_TokOutPort;
    Atelier                             *m_Atelier; 
    Bulwark                             m_Bulwark;
    bool                                m_CloseFlg;

    Sg_AtelierEasel( const std::string &name = "Atelier") 
        : Base( name), m_CloseFlg( false)
    {}

    //_____________________________________________________________________________________________________________________________

    bool    DoInit( Vita *vita)
    {
        if ( !Base::DoInit( vita))
            return false;  

        if ( vita->m_ImgFile.size())  
            m_Bulwark.Setup( vita->m_Atelier);  
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

        if ( !szBurst && wharf.IsClose() && (( m_CloseFlg = true)) && wharf.SetClose())
            return;
        
        if ( szBurst > szTokBurst)
            szBurst = szTokBurst;
        if ( !szBurst)
        {
            stats->m_ChokeSz.Incr(); 
            wharf.SetSize( 0);
            tokWharf.SetSize( 0);
            return;
        } 
        uint32_t    tokInd = 0;
        uint32_t    dInd = 0;
        uint32_t    tokCnt = 0;
        for ( ; dInd < szBurst;  dInd++)
        {
            if ( m_Bulwark.m_TokenSet && ( m_Bulwark.m_TokenSet->Size() >  TokenGram::Sz/2))
            {    
                tokCnt += m_Bulwark.m_TokenSet->SzFill();
                if ( tokWharf.IsTail()) 
                    tokWharf.Discard( m_Bulwark.m_TokenSet);
                else
                    tokWharf.Set( tokInd, m_Bulwark.m_TokenSet);
                tokInd++;
                m_Bulwark.m_TokenSet = NULL;
            }
            if ( !m_Bulwark.m_TokenSet)
                m_Bulwark.m_TokenSet = tokWharf.AllocFree();
            Datagram        *datagram = wharf.Get( dInd); 
            for ( uint32_t k = 0; k < datagram->SzFill(); ++k)
            {
                uint8_t     chr = datagram->At( k);
                
                bool        proceed = m_Bulwark.Play( chr);
            }
            if ( wharf.IsTail()) 
                wharf.Discard( datagram);
        }
        if ( m_Bulwark.m_TokenSet)
        {   
            tokCnt += m_Bulwark.m_TokenSet->SzFill();
            if ( tokWharf.IsTail()) 
                tokWharf.Discard( m_Bulwark.m_TokenSet);
            else
                tokWharf.Set( tokInd, m_Bulwark.m_TokenSet);
            tokInd++;
            m_Bulwark.m_TokenSet = NULL;

        }
        wharf.SetSize( dInd);
        tokWharf.SetSize( tokInd);
        stats->m_Matches += tokCnt;
        return;
    }
}; 
//_____________________________________________________________________________________________________________________________


struct  Cv_TokenLogStats : public Cv_EaselStats
{
    typedef Cv_EaselStats       Base;
 
    void    LogStats( std::ostream &strm, Cv_TokenLogStats *prev)
    {
        Base::LogStats( strm, prev); 
        return;
    }
};
//_____________________________________________________________________________________________________________________________

template < typename Vita>
struct Sg_TokenLogEasel : public Sg_WorkEasel< Sg_TokenLogEasel< Vita>, Vita, Cv_TokenLogStats>
{
    typedef Sg_WorkEasel< Sg_TokenLogEasel< Vita>, Vita, Cv_TokenLogStats>       Base;

    typedef typename Vita::TokenGram            TokenGram;    
    typedef typename Vita::InTokPort            InTokPort;
    typedef typename InTokPort::Wharf           InTokWharf; 
    typedef typename Base::Stats                Stats;

    Cv_File             m_OutFile;
    InTokPort           m_InTokPort;

    Sg_TokenLogEasel( const std::string &name = "TokenLog") 
        : Base( name)
    {}

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
        return m_OutFile.IsActive();
    }

    void    DoRunStep( void)
    {   
        Stats           *stats = this->CurStats();
        InTokWharf          wharf( &m_InTokPort);
        uint32_t        szBurst = wharf.Size(); 

        if ( !szBurst)
        {
            stats->m_ChokeSz.Incr();
            if ( wharf.IsClose())
            {
                m_OutFile.Shut(); 
                wharf.SetClose();
            }
            return;
        }
        std::stringstream   sstrm;
        for ( uint32_t i = 0; i < szBurst;  i++)
        {   
            TokenGram   *tokengram = wharf.Get( i); 
            uint32_t    szWrite = tokengram->SzFill(); // m_OutFile.Write( datagram->PtrAt( 0), datagram->SzFill());  
            for ( uint32_t k = 0; k < szWrite; ++k)
                sstrm << tokengram->At( k);
            wharf.Discard( tokengram); 
        }
        m_OutFile.Write( sstrm.str().c_str(), uint32_t( sstrm.str().length()));
        return;
    }
}; 

//_____________________________________________________________________________________________________________________________
