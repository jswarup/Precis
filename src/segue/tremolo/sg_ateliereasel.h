// sg_ateliereasel.h ___________________________________________________________________________________________________________________
#pragma once

#include    "segue/tremolo/sg_atelier.h" 

//_____________________________________________________________________________________________________________________________

struct Sg_AtelierVita  
{    
    typedef Cv_Array< uint8_t, 256>                 Datagram; 
    typedef Sg_DataSink< Datagram, 64, 4096>        OutPort; 
    typedef Sg_DataSource< OutPort>                 InPort;

    std::string             m_ImgFile;
    std::string             m_InputFile;
    std::string             m_OutputFile;
    std::string             m_RuleFile;
 
};

//_____________________________________________________________________________________________________________________________

template < typename Vita>
struct Sg_AtelierEasel : public Sg_WorkEasel< Sg_AtelierEasel< Vita>, Vita>
{
    typedef Sg_AtelierVita::Datagram          Datagram;
    typedef Sg_AtelierVita::InPort            InPort;
    typedef typename InPort::Wharf              Wharf;

    InPort                  m_DataPort;
    Sg_DfaReposAtelier      *m_DfaReposAtelier;
    Sg_DfaBlossomAtelier    *m_DfaBlossomAtelier;
    FsaDfaRepos             m_DfaRepos;
    Sg_Bulwark              m_Bulwark;
    bool                    m_CloseFlg;
    std::vector< uint8_t>   m_MemArr;

    Sg_AtelierEasel( void) 
        : m_DfaReposAtelier( NULL), m_DfaBlossomAtelier( NULL), m_CloseFlg( false)
    {}

    //_____________________________________________________________________________________________________________________________

    bool    DoInit( Vita *vita)
    {
        if ( !Sg_BaseEasel::DoInit( vita))
            return false; 
/*
        if ( vita->m_RuleFile.size())
        {
            StrInStream			    memVector;
            bool	                res = Cv_Aid::ReadVec( memVector.CharVec(), vita->m_RuleFile.c_str()); 
            if ( !res)
            {
                std::cerr << "Not Found : " << vita->m_RuleFile << '\n';
                return false;
            }
            Parser< StrInStream>	parser( &memVector);   
            RExpRepos				rexpRepos;
            RExpDoc					rexpDoc; 
            RExpDoc::XAct           xact( &rexpRepos); 
            bool					apiErrCode = parser.Match( &rexpDoc, &xact); 

            FsaElemRepos            elemRepos;
            FsaElemReposCnstr       automReposCnstr(  &rexpRepos, &elemRepos); 
            automReposCnstr.Process();   
            FsaDfaCnstr             dfaCnstr( &elemRepos, &m_DfaRepos); 
            dfaCnstr.SubsetConstruction();
            if ( 0) {
                m_DfaReposAtelier = new Sg_DfaReposAtelier( &m_DfaRepos);
                m_DfaRepos.m_DistribRepos.Dump( std::cout);
                std::ofstream           fsaOStrm( "a.dot");
                Cv_DotStream			fsaDotStrm( &fsaOStrm, true);  
                m_DfaRepos.WriteDot( fsaDotStrm);
            }
            {
                Cv_FileSpritz           imgSpritz( vita->m_ImgFile, Cv_FileSpritz::WriteTrim); 
                Cv_ValidationSpritz     valSpritz( &imgSpritz); 

                Cv_Aid::Save( &valSpritz, m_DfaRepos);
                //Cv_Aid::Save( &valSpritz, &dfaRepos.m_DistribRepos);
                bool t = true;
            }
        }
*/
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
        InPort::Wharf   wharf( &m_DataPort);
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
