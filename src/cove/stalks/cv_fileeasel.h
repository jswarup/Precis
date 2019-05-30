// cv_fileeasel.h ___________________________________________________________________________________________________________________
#pragma once


#include    "cove/stalks/cv_workeasel.h" 

//_____________________________________________________________________________________________________________________________

template < typename Vita>
struct Sg_FileReadEasel : public Sg_WorkEasel< Sg_FileReadEasel< Vita>, Vita>
{
    typedef Sg_WorkEasel< Sg_FileReadEasel< Vita>, Vita>    Base;
    typedef typename Vita::Datagram          Datagram;
    typedef typename Vita::OutPort           OutPort;
    typedef typename OutPort::Wharf         OutWharf;

    Cv_File         m_InFile;
    OutPort         m_DataPort;
    bool            m_FileClosingFlg;
    uint32_t        m_CharIndex; 

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

    bool    IsRunable( void)
    {
        return m_InFile.IsActive();
    }

    void    DoRunStep( void)
    {  
        OutWharf        wharf( &m_DataPort);
        if ( m_FileClosingFlg)
        {
            if ( m_Vita->m_InputLoopFlg )
            {
                m_FileClosingFlg = false;
                m_InFile.Rewind();
            }
            else if ( m_InFile.Shut())
                wharf.SetClose();
                
            return;
        }
        uint32_t        szBurst = wharf.Size(); 
        szBurst = wharf.ProbeSzFree( szBurst);
        uint32_t        dInd = 0;
        for ( ; dInd < szBurst;  dInd++)
        {   
            Datagram    *datagram = wharf.AllocFree();
            uint32_t    szFill = m_InFile.Read( datagram->PtrAt( 0), datagram->SzVoid());
            datagram->MarkFill( szFill);
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
struct Sg_FileWriteEasel : public Sg_WorkEasel< Sg_FileWriteEasel< Vita>, Vita>
{
    typedef Sg_WorkEasel< Sg_FileWriteEasel< Vita>, Vita>   Base;
    typedef typename Vita::Datagram                         Datagram;
    typedef typename Vita::InPort                           InPort;
    typedef typename InPort::Wharf                          InWharf;

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
        InWharf         wharf( &m_DataPort);
        uint32_t        szBurst = wharf.Size(); 

        if ( !szBurst && wharf.IsClose() && m_OutFile.Shut() && wharf.SetClose())
            return;

        for ( uint32_t i = 0; i < szBurst;  i++)
        {   
            Datagram    *datagram = wharf.Get( i); 
            uint32_t    szWrite = m_OutFile.Write( datagram->PtrAt( 0), datagram->SzFill());  
            wharf.Discard( datagram); 
        }
        return;
    }
}; 


//_____________________________________________________________________________________________________________________________
