// cv_fileeasel.h ___________________________________________________________________________________________________________________
#pragma once


#include    "cove/stalks/cv_workeasel.h" 

//_____________________________________________________________________________________________________________________________


struct  Cv_FileStats : public Cv_EaselStats
{
    typedef Cv_EaselStats       Base;

    Cv_Type< uint64_t>          m_Bytes;

    uint64_t    Bytes( Cv_FileStats *prev) { return m_Bytes.Get() -prev->m_Bytes.Get(); }

    void    LogStats( std::ostream &strm, Cv_FileStats *prev)
    {
        Base::LogStats( strm, prev);
        strm << "MBytes[ " << double( Bytes( prev))/( 1024*1024) << "] ";
        return;
    }
};

//_____________________________________________________________________________________________________________________________

template < typename Easel, typename Vita>
struct Sg_FileReadEasel : public Sg_WorkEasel< Easel, Vita, Cv_FileStats>
{
    typedef Sg_WorkEasel< Easel, Vita, Cv_FileStats>    Base;
    typedef typename Vita::Datagram         Datagram;
    typedef typename Vita::OutPort          OutPort;
    typedef typename OutPort::Wharf         OutWharf;
    typedef typename Base::Stats            Stats;

    Cv_File                     m_InFile;
    OutPort                     m_DataPort;
    bool                        m_FileClosingFlg;
    uint32_t                    m_CharIndex;  

    Sg_FileReadEasel( const std::string &name = "FileRead") 
        : Base( name), m_FileClosingFlg( false), m_CharIndex( 0) 
    {}


    bool    DoInit( Vita *vita)
    {
        if ( !Base::DoInit( vita))
            return false;

        if ( !m_InFile.Open( vita->m_InputFile.c_str(), true))
            return false;
        return true;
    }

    void  ProcessDatagram( Datagram *dgram)
    {
    }
 
    bool    IsRunable( void)
    {
        return m_InFile.IsActive();
    }

    void    DoRunStep( void)
    {  
        Stats           *stats = this->CurStats();
        if ( m_FileClosingFlg && this->m_Vita->m_InputLoopFlg )
        {
            m_FileClosingFlg = false;
            m_InFile.Rewind();
            return;
        }
        OutWharf        wharf( &m_DataPort);
        
        if ( m_FileClosingFlg && m_InFile.Shut())
        {
            wharf.SetClose();
            return;
        }
        uint32_t        szBurst = wharf.Size(); 
        szBurst = wharf.ProbeSzFree( szBurst);
        if ( !szBurst) 
            stats->m_ChokeSz.Incr(); 

        Easel           *thisEasel = ( Easel *) this;
        uint32_t        dInd = 0;
        for ( ; dInd < szBurst;  dInd++)
        {   
            Datagram    *datagram = wharf.AllocFree();
            uint32_t    szFill = m_InFile.Read( datagram->PtrAt( 0), datagram->SzVoid());
            datagram->MarkFill( szFill);
            thisEasel->ProcessDatagram( datagram);
            stats->m_Bytes.Incr( szFill);

            if ( !szFill || wharf.IsTail()) 
                wharf.Discard( datagram);
            else {
                wharf.Set( dInd, datagram); 
            }
            if ( !szFill)
            {
                m_FileClosingFlg = true;
                break;
            }
        }
        wharf.SetSize( dInd);
        return;
    }
}; 

//_____________________________________________________________________________________________________________________________

template < typename Vita>
struct Sg_FileWriteEasel : public Sg_WorkEasel< Sg_FileWriteEasel< Vita>, Vita, Cv_FileStats>
{
    typedef Sg_WorkEasel< Sg_FileWriteEasel< Vita>, Vita, Cv_FileStats>   Base;
    typedef typename Vita::Datagram                         Datagram;
    typedef typename Vita::InPort                           InPort;
    typedef typename InPort::Wharf                          InWharf;
    typedef typename Base::Stats                            Stats;

    Cv_File         m_OutFile;
    InPort          m_DataPort;

    Sg_FileWriteEasel( const std::string &name = "FileWrite") 
        : Base( name)
    {}

    bool    DoInit( Vita *vita)
    {
        if ( !Base::DoInit( vita))
            return false; 
        if ( !m_OutFile.Open( vita->m_OutputFile.c_str(), false))
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
        InWharf         wharf( &m_DataPort);
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

        for ( uint32_t i = 0; i < szBurst;  i++)
        {   
            Datagram    *datagram = wharf.Get( i); 
            uint32_t    szWrite = m_OutFile.Write( datagram->PtrAt( 0), datagram->SzFill());  

            stats->m_Bytes.Incr( szWrite);
            wharf.Discard( datagram); 
        }
        return;
    }
}; 


//_____________________________________________________________________________________________________________________________
