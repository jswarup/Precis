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
        InTokWharf      wharf( &m_InTokPort);
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
