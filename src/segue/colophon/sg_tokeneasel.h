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
        m_InTokPorts.MarkFill( 1);
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
 

    Cv_Array< InTokWharf, SzPort>      Wharfs( void)
    {
        Cv_Array< InTokWharf, SzPort>   wharfs;
        wharfs.MarkFill( m_InTokPorts.SzFill());
        for ( uint32_t i = 0; i < wharfs.SzFill(); ++i)
            wharfs[ i].Load( &m_InTokPorts[ i]);
        return wharfs; 
    }

    void    DoRunStep( void)
    {   
        Stats                           *stats = this->CurStats();
        Cv_Array< InTokWharf, SzPort>   wharfs = Wharfs();
        uint32_t                        closeCount = 0;
        for ( uint32_t i = 0; i < m_InTokPorts.SzFill(); ++i)
        {
            InTokWharf      &wharf = wharfs[ i];
            uint32_t        szBurst = wharf.Size(); 

            if ( !szBurst)
            {
                stats->m_ChokeSz.Incr();
                if ( wharf.IsPrevClose())
                {
                    ++closeCount;
                    wharf.SetClose();
                }
                continue;
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
        }
        if ( closeCount == m_InTokPorts.SzFill())
            m_OutFile.Shut(); 
            
        return;
    }
}; 

//_____________________________________________________________________________________________________________________________
