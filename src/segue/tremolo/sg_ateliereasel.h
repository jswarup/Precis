// sg_ateliereasel.h ___________________________________________________________________________________________________________________
#pragma once

#include    "segue/tremolo/sg_atelier.h" 
 

//_____________________________________________________________________________________________________________________________

template < typename Vita>
struct Sg_AtelierEasel : public Sg_WorkEasel< Sg_AtelierEasel< Vita>, Vita>
{
    typedef Sg_WorkEasel< Sg_AtelierEasel< Vita>, Vita>     Base;

    typedef typename Vita::Datagram     Datagram;
    typedef typename Vita::InPort       InPort;
    typedef typename InPort::Wharf      Wharf;

    InPort                              m_DataPort;
    Sg_DfaReposAtelier                  *m_DfaReposAtelier;
    Sg_DfaBlossomAtelier                *m_DfaBlossomAtelier;
    FsaDfaRepos                         m_DfaRepos;
    Sg_Bulwark                          m_Bulwark;
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
        Wharf           wharf( &m_DataPort);
        uint32_t        szBurst = wharf.Size(); 

        if ( !szBurst && wharf.IsClose() && (( m_CloseFlg = true)) && wharf.SetClose())
            return;

        uint32_t        dInd = 0;
        for ( ; dInd < szBurst;  dInd++)
        {
            Datagram    *datagram = wharf.Get( dInd); 
            for ( uint32_t k = 0; k < datagram->SzFill(); ++k)
            {
                uint8_t     chr = datagram->At( k);
                m_Bulwark.Play( m_DfaBlossomAtelier, chr);
            }
        }
        wharf.SetSize( dInd);
        return;
    }
}; 

//_____________________________________________________________________________________________________________________________
