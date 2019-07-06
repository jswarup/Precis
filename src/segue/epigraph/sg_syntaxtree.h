//  sg_syntaxtree.h ___________________________________________________________________________________________________________________
#pragma once

#include 	"cove/silo/cv_cstr.h"
#include 	"cove/silo/cv_craterepos.h" 
#include 	"cove/flux/cv_dotstream.h" 
#include    "segue/epigraph/sg_chset.h"

//_____________________________________________________________________________________________________________________________  

namespace Sg_Timbre
{
    
struct      SynElem;
struct      RefSynElem;
struct      ActionSynElem;
struct      BOLSynElem;
struct      SeqSynElem;
struct      AltSynElem;
struct      RepeatSynElem;
struct      RefSynElem;
struct	    LexemeSynElem;
struct	    ErrorSynElem;
struct      Cv_SynCrate; 

//_____________________________________________________________________________________________________________________________ 

typedef Cv_Crate< RefSynElem, RepeatSynElem, AltSynElem, SeqSynElem, ActionSynElem, BOLSynElem, RefSynElem, LexemeSynElem, ErrorSynElem, SynElem>   SynCrate; 
typedef Cv_CrateRepos< SynCrate>                                                                                                        SynRepos;

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

    bool    WriteDot( uint32_t id, Cv_DotStream &strm)  
    {
        return true;
    } 	 
};   

//_____________________________________________________________________________________________________________________________ 

struct     RefSynElem : public SynElem 
{
    SynRepos::Id		m_Elem;
    
	RefSynElem( void)
	{}

	std::string		GetName( void) const { return Cv_Aid::ToStr( "Ref"); }

    bool    WriteDot( uint32_t id, Cv_DotStream &strm)  
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
    SynRepos::Id		m_Elem;

	std::string		GetName( void) const { return Cv_Aid::ToStr( "Lex"); }

    bool    WriteDot( uint32_t id, Cv_DotStream &strm)  
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

	std::string		GetName( void) const { return Cv_Aid::ToStr( "Error"); }

	bool    WriteDot( uint32_t id, Cv_DotStream &strm)  
	{
		strm << 'R' << m_IPtr << " [ shape=parallelogram  label= <<FONT> #" << GetName() << " " << m_ErrStr << " " << "<BR />" ; 
		strm << " </FONT>>];\n "; 
		 
		return true;
	} 

};


//_____________________________________________________________________________________________________________________________ 

struct     BOLSynElem : public SynElem 
{ 

    std::string		GetName( void) const { return Cv_Aid::ToStr( "BOL"); }

    bool    WriteDot( uint32_t id, Cv_DotStream &strm)  
    {
        strm << 'R' << m_IPtr << " [ shape=parallelogram  label= <<FONT> #" << GetName() << " " << "BOL" << " " << "<BR />" ; 
        strm << " </FONT>>];\n "; 

        return true;
    } 

};

//_____________________________________________________________________________________________________________________________ 

struct     ActionSynElem : public SynElem 
{ 
    SynRepos::Id    m_Elem;
    uint64_t        m_Token;

    ActionSynElem( void)
        : m_Token( 0)
    {}

	std::string		GetName( void) const { return Cv_Aid::ToStr( "Act"); } 

    bool    WriteDot( uint32_t id, Cv_DotStream &strm)   
    {
        strm << 'R' << m_IPtr << " [ shape=box  label= <<FONT> #" << GetName() << "<BR />" ; 
        strm << " </FONT>>];\n "; 
 
        strm << 'R' << m_IPtr << " -> " << 'R' << m_Elem.m_IPtr << " [ arrowhead=tee color=black] ; \n"; 
        return true;
    } 
     
};

//_____________________________________________________________________________________________________________________________ 

struct     SeqSynElem : public SynElem 
{
    std::vector< SynRepos::Id>     m_SeqList; 

	std::string		GetName( void) const { return Cv_Aid::ToStr( "Seq"); }  
    
    bool			WriteDot( uint32_t id, Cv_DotStream &strm)  
    {
        strm << 'R' << m_IPtr << " [ shape=rectangle  label= <<FONT> #" << GetName() << "<BR />" ; 
        strm << " </FONT>>];\n "; 
		 
        IPtrStor    tailIPtr = m_IPtr;
        for ( uint32_t k = 0; k < m_SeqList.size(); ++k)
        {
            SynRepos::Id		regex = m_SeqList[ k];
            strm << 'R' << tailIPtr << " -> " << 'R' << regex.m_IPtr;
            if ( tailIPtr == m_IPtr)
                strm << " [ arrowhead=tee color=black] ; \n"; 
            else
                strm << " [ arrowhead=vee color=black] ; \n"; 
            tailIPtr = regex.m_IPtr;
        }
        return true;
    } 
};

//_____________________________________________________________________________________________________________________________ 

struct     AltSynElem : public SynElem 
{
    std::vector< SynRepos::Id>     m_AltList; 
	 
	std::string		GetName( void) const { return Cv_Aid::ToStr( "Alt"); }  

    bool    WriteDot( uint32_t id, Cv_DotStream &strm)  
    {
        strm << 'R' << m_IPtr << " [ shape=parallelogram  label= <<FONT> #" << GetName() << "<BR />" ; 
        strm << " </FONT>>];\n "; 

        for ( uint32_t k = 0; k < m_AltList.size(); ++k)
        {
            SynRepos::Id		regex = m_AltList[ k];
            strm << 'R' << m_IPtr << " -> " << 'R' << regex.m_IPtr << " [ arrowhead=tee color=black] ; \n";
        }
        return true;
    }
 
};

//_____________________________________________________________________________________________________________________________ 

struct     RepeatSynElem : public SynElem 
{
    SynRepos::Id	    m_Elem;
    uint32_t            m_Min;
    uint32_t            m_Max;
	 
	std::string		GetName( void) const { return Cv_Aid::ToStr( "Rep"); } 

    bool    WriteDot( uint32_t id, Cv_DotStream &strm)  
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