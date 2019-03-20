// sg_automelem.cpp ___________________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/tremolo/sg_automelem.h"

using namespace Sg_RExp;

//_____________________________________________________________________________________________________________________________  

bool  AutomElem::WriteDot( Cv_DotStream &strm)  
{
    strm << 'R' << GetId() << " [ shape=";

    if ( m_Action)
        strm << "box";
    else
        strm << "ellipse";
    strm << " color=cyan label= <<FONT> N" << GetId() << "<BR />" << RefCount() << "<BR />" ;
    if ( m_Action)
        strm << 'T' << m_Action->m_Value;
    strm << " </FONT>>];\n "; 

    Cv_CArr< FsaVar>    dests = Dests(); 
    for ( uint32_t k = 0; k < dests.Size(); ++k)
    {
        AutomElem      *regex = ( AutomElem *) dests[ k];
        strm << 'R' << GetId() << " -> " << 'R' << regex->GetId() << " [ arrowhead=normal color=black label=<<FONT> ";  
        strm << Cv_Aid::XmlEncode(  m_ChSets[ k].ToString());
        strm << "</FONT>>] ; \n" ;  
    }
    return true;
}

//_____________________________________________________________________________________________________________________________  

