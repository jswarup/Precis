//  sg_syntaxtree.h ___________________________________________________________________________________________________________________
#pragma once

#include 	"cove/silo/cv_cstr.h"
#include 	"cove/silo/cv_crate.h" 
#include 	"cove/silo/cv_dotstream.h" 
#include    "segue/timbre/sg_chset.h"

//_____________________________________________________________________________________________________________________________  

namespace Sg_Timbre
{
    
struct     SynElem;
struct     RefSynElem;
struct     ActionSynElem;
struct     SeqSynElem;
struct     AltSynElem;
struct     RepeatSynElem;
struct     RefSynElem;
struct	   LexemeSynElem;
struct	   ErrorSynElem;
struct     Cv_SynCrate; 

//_____________________________________________________________________________________________________________________________ 

typedef Cv_Crate< RefSynElem, RepeatSynElem, AltSynElem, SeqSynElem, ActionSynElem, RefSynElem, LexemeSynElem, ErrorSynElem, SynElem>   SynCrate; 

//_____________________________________________________________________________________________________________________________ 
    
struct  SynElem   : public Cv_CrateEntry 
{
	bool	m_LockFlg;

public:
	SynElem( void) 
		: m_LockFlg( false)
	{}

	bool			IsLocked( void) const { return m_LockFlg; }

    std::string		GetName( void) const { return "Syn"; } 

    bool    WriteDot( Cv_DotStream &strm)  
    {
        return true;
    } 	 
};   

//_____________________________________________________________________________________________________________________________ 

struct     RefSynElem : public SynElem 
{
    Cv_CrateId		m_Elem;
    
	RefSynElem( void)
	{}

	std::string		GetName( void) const { return Cv_Aid::ToStr( "Ref", GetId()); }

    bool    WriteDot( Cv_DotStream &strm)  
    {
        strm << 'R' << m_IPtr << " [ shape=parallelogram  label= <<FONT> #" << GetName() << "<BR />" ; 
        strm << " </FONT>>];\n "; 
 
        strm << 'R' << m_IPtr << " -> " << 'R' << m_Elem.m_IPtr << " [ arrowhead=tee color=black] ; \n"; 
        return true;
    } 
 
};

//_____________________________________________________________________________________________________________________________ 

struct     LexemeSynElem : public SynElem 
{
	Cv_CrateId		m_Elem;

	std::string		GetName( void) const { return Cv_Aid::ToStr( "Lex", GetId()); }

    bool    WriteDot( Cv_DotStream &strm)  
    {
        strm << 'R' << m_IPtr << " [ shape=parallelogram  label= <<FONT> #" << GetName() << "<BR />" ; 
        strm << " </FONT>>];\n "; 

        strm << 'R' << m_IPtr << " -> " << 'R' << m_Elem.m_IPtr << " [ arrowhead=tee color=black] ; \n"; 
        return true;
    } 

};

//_____________________________________________________________________________________________________________________________ 

struct     ErrorSynElem : public SynElem 
{
	std::string     m_ErrStr;

	std::string		GetName( void) const { return Cv_Aid::ToStr( "Lex", GetId()); }

	bool    WriteDot( Cv_DotStream &strm)  
	{
		strm << 'R' << m_IPtr << " [ shape=parallelogram  label= <<FONT> #" << GetName() << " " << m_ErrStr << " " << "<BR />" ; 
		strm << " </FONT>>];\n "; 
		 
		return true;
	} 

};
//_____________________________________________________________________________________________________________________________ 

struct     ActionSynElem : public RefSynElem 
{ 

	std::string		GetName( void) const { return Cv_Aid::ToStr( "Act", GetId()); } 

    bool    WriteDot( Cv_DotStream &strm)   
    {
        strm << 'R' << m_IPtr << " [ shape=parallelogram  label= <<FONT> #" << GetName() << "<BR />" ; 
        strm << " </FONT>>];\n "; 
 
        strm << 'R' << m_IPtr << " -> " << 'R' << m_Elem.m_IPtr << " [ arrowhead=tee color=black] ; \n"; 
        return true;
    } 
    
 
};

//_____________________________________________________________________________________________________________________________ 

struct     SeqSynElem : public SynElem 
{
    std::vector< Cv_CrateId>     m_SeqList; 

	std::string		GetName( void) const { return Cv_Aid::ToStr( "Seq", GetId()); }  
    
    bool			WriteDot( Cv_DotStream &strm)  
    {
        strm << 'R' << m_IPtr << " [ shape=rectangle  label= <<FONT> #" << GetName() << "<BR />" ; 
        strm << " </FONT>>];\n "; 
		 
        for ( uint32_t k = 0; k < m_SeqList.size(); ++k)
        {
			Cv_CrateId		regex = m_SeqList[ k];
            strm << 'R' << m_IPtr << " -> " << 'R' << regex.m_IPtr << " [ arrowhead=tee color=black] ; \n"; 
        }
        return true;
    }
    
 
};

//_____________________________________________________________________________________________________________________________ 

struct     AltSynElem : public SynElem 
{
    std::vector< Cv_CrateId>     m_AltList; 
	 
	std::string		GetName( void) const { return Cv_Aid::ToStr( "Alt", GetId()); }  

    bool    WriteDot( Cv_DotStream &strm)  
    {
        strm << 'R' << m_IPtr << " [ shape=parallelogram  label= <<FONT> #" << GetName() << "<BR />" ; 
        strm << " </FONT>>];\n "; 

        for ( uint32_t k = 0; k < m_AltList.size(); ++k)
        {
			Cv_CrateId		regex = m_AltList[ k];
            strm << 'R' << m_IPtr << " -> " << 'R' << regex.m_IPtr << " [ arrowhead=tee color=black] ; \n";
        }
        return true;
    }
 
};

//_____________________________________________________________________________________________________________________________ 

struct     RepeatSynElem : public SynElem 
{
    Cv_CrateId			m_Elem;
    uint32_t            m_Min;
    uint32_t            m_Max;
	 
	std::string		GetName( void) const { return Cv_Aid::ToStr( "Rep", GetId()); } 

    bool    WriteDot( Cv_DotStream &strm)  
    {
        strm << 'R' << m_IPtr << " [ shape=hexagon  label= <<FONT> #" << GetName() << "  <BR />";
        strm.OStream() << "( " << uint32_t( m_Min) << ", ";
        if ( m_Max == CV_UINT32_MAX)
            strm << "Inf";
        else
            strm << m_Max;
        strm  << ")" << " </FONT>>];\n "; 

        strm << 'R' << m_IPtr << " -> " << 'R' << m_Elem.m_IPtr << " [ arrowhead=dot  color=black] ; \n"; 
        return true;
    }

 

};

//_____________________________________________________________________________________________________________________________ 

 
 
//_____________________________________________________________________________________________________________________________ 

}; 