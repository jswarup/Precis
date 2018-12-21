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
struct     Cv_SynCrate; 

//_____________________________________________________________________________________________________________________________ 

typedef Cv_Crate< RefSynElem, RepeatSynElem, AltSynElem, SeqSynElem, ActionSynElem, RefSynElem, SynElem>   SynCrate; 

//_____________________________________________________________________________________________________________________________ 
    
struct  SynElem   : public Cv_CrateEntry 
{
public:
	

    const char      *GetName( void) const { return "Syn"; } 

    bool    WriteDot( Cv_DotStream &strm)  
    {
        return true;
    } 	 
};   

//_____________________________________________________________________________________________________________________________ 

struct     RefSynElem : public SynElem 
{
    Cv_CrateId		m_Elem;
    
    const char      *GetName( void) const { return "Ref"; }

    bool    WriteDot( Cv_DotStream &strm)  
    {
        strm << 'R' << SELF << " [ shape=parallelogram  label= <<FONT> #" << SELF << "<BR />" ; 
        strm << " </FONT>>];\n "; 
 
        strm << 'R' << SELF << " -> " << 'R' << m_Elem << " [ arrowhead=tee color=black] ; \n"; 
        return true;
    } 
 
};

//_____________________________________________________________________________________________________________________________ 

struct     LexemeSynElem : public SynElem 
{
	Cv_CrateId		m_Elem;

    const char      *GetName( void) const { return "Ref"; }

    bool    WriteDot( Cv_DotStream &strm)  
    {
        strm << 'R' << SELF << " [ shape=parallelogram  label= <<FONT> #" << SELF << "<BR />" ; 
        strm << " </FONT>>];\n "; 

        strm << 'R' << SELF << " -> " << 'R' << m_Elem << " [ arrowhead=tee color=black] ; \n"; 
        return true;
    } 

};

//_____________________________________________________________________________________________________________________________ 

struct     ActionSynElem : public RefSynElem 
{ 

    const char      *GetName( void) const { return "Act"; }

    bool    WriteDot( Cv_DotStream &strm)   
    {
        strm << 'R' << SELF << " [ shape=parallelogram  label= <<FONT> #" << SELF << "<BR />" ; 
        strm << " </FONT>>];\n "; 
 
        strm << 'R' << SELF << " -> " << 'R' << m_Elem << " [ arrowhead=tee color=black] ; \n"; 
        return true;
    } 
    
 
};

//_____________________________________________________________________________________________________________________________ 

struct     SeqSynElem : public SynElem 
{
    std::vector< Cv_CrateId>     m_SeqList; 

    const char                  *GetName( void) const { return "Seq"; } 
    
    bool    WriteDot( Cv_DotStream &strm)  
    {
        strm << 'R' << SELF << " [ shape=parallelogram  label= <<FONT> #" << SELF << "<BR />" ; 
        strm << " </FONT>>];\n "; 

		Cv_CrateId	fRegex = SELF;
        for ( uint32_t k = 0; k < m_SeqList.size(); ++k)
        {
			Cv_CrateId		regex = m_SeqList[ k];
            strm << 'R' << fRegex << " -> " << 'R' << regex << " [ arrowhead=tee color=black] ; \n";
            regex = fRegex;
        }
        return true;
    }
    
 
};

//_____________________________________________________________________________________________________________________________ 

struct     AltSynElem : public SynElem 
{
    std::vector< Cv_CrateId>     m_AltList; 

    const char      *GetName( void) const { return "Alt"; } 

    bool    WriteDot( Cv_DotStream &strm)  
    {
        strm << 'R' << SELF << " [ shape=parallelogram  label= <<FONT> #" << SELF << "<BR />" ; 
        strm << " </FONT>>];\n "; 

        for ( uint32_t k = 0; k < m_AltList.size(); ++k)
        {
			Cv_CrateId		regex = m_AltList[ k];
            strm << 'R' << SELF << " -> " << 'R' << regex << " [ arrowhead=tee color=black] ; \n";
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

    const char      *GetName( void) const { return "Rep"; }

    bool    WriteDot( Cv_DotStream &strm)  
    {
        strm << 'R' << SELF << " [ shape=hexagon  label= <<FONT> #" << SELF << "  <BR />";
        strm.OStream() << "( " << uint32_t( m_Min) << ", ";
        if ( m_Max == CV_UINT32_MAX)
            strm << "Inf";
        else
            strm << m_Max;
        strm  << ")" << " </FONT>>];\n "; 

        strm << 'R' << SELF << " -> " << 'R' << m_Elem << " [ arrowhead=dot  color=black] ; \n"; 
        return true;
    }

 

};

//_____________________________________________________________________________________________________________________________ 

 
 
//_____________________________________________________________________________________________________________________________ 

}; 