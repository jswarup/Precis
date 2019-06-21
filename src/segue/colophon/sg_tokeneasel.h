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
    std::ostream                                m_OutStream;
    Cv_SpritzArray< 8096>                       m_SpritzArray;
    
    Sg_TokenLogEasel( const std::string &name = "TokenLog") 
        : Base( name) , m_OutStream( &m_SpritzArray)
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

    void    DoRunStep( void)
    {   
        Stats                           *stats = this->CurStats(); 
        uint32_t                        closeCount = 0; 
        m_SpritzArray.Reset();
        for ( uint32_t i = 0; i < m_InTokPorts.SzFill(); ++i)
        {
            InTokWharf      wharf( &m_InTokPorts[ i]);
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
            
            for ( uint32_t i = 0; i < szBurst;  i++)
            {   
                TokenGram   *tokengram = wharf.Get( i); 
                uint32_t    szWrite = tokengram->SzFill();   
                for ( uint32_t k = 0; k < szWrite; ++k)
                    m_OutStream << tokengram->At( k);
            }   
            for ( uint32_t i = 0; i < szBurst;  i++)
                wharf.Discard( wharf.Get( i)); 
            
            wharf.SetSize( szBurst);
        }
        if ( m_SpritzArray.SzFill())
            m_OutFile.Write( m_SpritzArray.PtrAt( 0), m_SpritzArray.SzFill()); 
        if ( closeCount == m_InTokPorts.SzFill())
            m_OutFile.Shut();             
        return;
    }
}; 

//_____________________________________________________________________________________________________________________________
