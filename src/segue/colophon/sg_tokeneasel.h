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
        SzSpritz = 4096*4,
        SzPort = 8 
    };
     
    typedef Sg_WorkEasel< Sg_TokenLogEasel< Vita>, Vita, Cv_TokenLogStats>       Base;

    typedef typename Vita::TokenGram            TokenGram;    
    typedef typename Vita::InTokPort            InTokPort;
    typedef typename InTokPort::Wharf           InTokWharf; 
    typedef typename Base::Stats                Stats;
    
    Cv_File                                     m_OutFile;
    Cv_Array< InTokPort, SzPort>                m_InTokPorts;  
    Cv_SpritzBuf< SzSpritz>                     m_SpritzArray;
    
    Sg_TokenLogEasel( const std::string &name = "TokenLog") 
        : Base( name)
    {
        m_SpritzArray.SetFile( &m_OutFile);
    }
 
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
  
    uint32_t LoadInTokWharfs(  Cv_Array< InTokWharf, SzPort> *inWharfs)
    { 
        Stats               *stats = this->CurStats(); 
        uint32_t            closeCount = 0; 

        for ( uint32_t i = 0; i < m_InTokPorts.SzFill(); ++i)
        {
            InTokWharf      *wharf = inWharfs->PtrAt( inWharfs->SzFill());
            wharf->Load( &m_InTokPorts[ i]);
            uint32_t        szBurst = wharf->Size();  
            if ( szBurst)
            {
                inWharfs->m_SzFill++;
                continue;
            }
            stats->m_ChokeSz.Incr();
            if ( wharf->IsPrevClose())
            {
                ++closeCount;
                wharf->SetClose();
            }
            wharf->Unload();             
        }
        return closeCount;
    } 
    
    void    DoRunStep( void)
    {   
        Stats                           *stats = this->CurStats(); 
        Cv_Array< InTokWharf, SzPort>   inWharfs;
        
        uint32_t                        closeCount = LoadInTokWharfs( &inWharfs);
        if ( closeCount == m_InTokPorts.SzFill())
        {
            m_SpritzArray.Flush();
            m_OutFile.Shut();             
            return;
        }   

        for ( uint32_t i = 0; i < inWharfs.SzFill(); ++i)
        {
            InTokWharf      *wharf = inWharfs.PtrAt( i);
            uint32_t        szBurst = wharf->Size();             
            for ( uint32_t i = 0; i < szBurst;  i++)
            {   
                TokenGram   *tokengram = wharf->Get( i); 
                tokengram->Dump( m_SpritzArray); 
                wharf->Discard( tokengram); 
            }    
            wharf->SetSize( szBurst);
        }
        
        return;
    }
}; 

//_____________________________________________________________________________________________________________________________
