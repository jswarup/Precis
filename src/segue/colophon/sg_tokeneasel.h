// sg_tokeneasel.h ___________________________________________________________________________________________________________________
#pragma once

#include    "segue/colophon/sg_atelier.h" 

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
    enum { 
        SzPort = 8 
    };

    typedef Sg_WorkEasel< Sg_TokenLogEasel< Vita>, Vita, Cv_TokenLogStats>       Base;

    typedef typename Vita::TokenGram            TokenGram;    
    typedef typename Vita::InTokPort            InTokPort;
    typedef typename InTokPort::Wharf           InTokWharf; 
    typedef typename Base::Stats                Stats;
    
    Cv_File                                     m_OutFile;
    Cv_Array< InTokPort, SzPort>                m_InTokPorts; 

    Sg_TokenLogEasel( const std::string &name = "TokenLog") 
        : Base( name) 
    {}
 
    uint32_t    Connect( typename Vita::OutTokPort *provider)
    {
        uint32_t    ind = m_InTokPorts.SzFill();
        m_InTokPorts.MarkFill( ind +1);
        m_InTokPorts[ ind].Connect( provider); 
        return ind;
    }

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

    void    DoRunService( InTokPort *inPort)
    {   
        Stats           *stats = this->CurStats();
        InTokWharf      wharf( &inPort);
        uint32_t        szBurst = wharf.Size(); 

        if ( !szBurst)
        {
            stats->m_ChokeSz.Incr();
            if ( wharf.IsPrevClose())
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
            uint32_t    szWrite = tokengram->SzFill();   
            for ( uint32_t k = 0; k < szWrite; ++k)
                sstrm << tokengram->At( k);
            wharf.Discard( tokengram); 
        }
        m_OutFile.Write( sstrm.str().c_str(), uint32_t( sstrm.str().length()));
        wharf.SetSize( szBurst);
        return;
    }

    void    DoRunStep( void)
    {   
        Stats           *stats = this->CurStats();
        InTokWharf      wharf( &m_InTokPorts[ 0]);
        uint32_t        szBurst = wharf.Size(); 

        if ( !szBurst)
        {
            stats->m_ChokeSz.Incr();
            if ( wharf.IsPrevClose())
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
            uint32_t    szWrite = tokengram->SzFill();   
            for ( uint32_t k = 0; k < szWrite; ++k)
                sstrm << tokengram->At( k);
            wharf.Discard( tokengram); 
        }
        m_OutFile.Write( sstrm.str().c_str(), uint32_t( sstrm.str().length()));
        wharf.SetSize( szBurst);
        return;
    }
}; 

//_____________________________________________________________________________________________________________________________
