//  sg_automelem.h ___________________________________________________________________________________________________________________
#pragma once

#include 	"cove/barn/cv_ptrslot.h" 
#include    "segue/tremolo/sg_filter.h"
#include 	"cove/silo/cv_repos.h"
#include 	"cove/silo/cv_dotstream.h"
#include    "segue/timbre/sg_distrib.h"

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{
//_____________________________________________________________________________________________________________________________ 

struct  AutomElem   : public Cv_ReposEntry, public Cv_Shared
{      
    std::vector< Sg_ChSet>          m_ChSets;
    std::vector< AutomElem *>      m_Dests;

    void        AddEdge( const Sg_ChSet &chSet, AutomElem *dest) 
    {
        m_ChSets.push_back( chSet);
        m_Dests.push_back( dest);
        dest->RaiseRef();
    } 

    bool        WriteDot( Cv_DotStream &strm)  
    {
        strm << 'R' << GetId() << " [ shape=ellipse color=cyan label= <<FONT> N" << GetId() << "<BR />" ; 
        strm << RefCount() << " </FONT>>];\n "; 

        for ( uint32_t k = 0; k < m_Dests.size(); ++k)
        {
            AutomElem      *regex = m_Dests[ k];
            strm << 'R' << GetId() << " -> " << 'R' << regex->GetId() << " [ arrowhead=normal color=black label=<<FONT> ";  
            strm << Cv_Aid::XmlEncode(  m_ChSets[ k].ToString());
            strm << "</FONT>>] ; \n" ;  
        }
        return true;
    }
};

//_____________________________________________________________________________________________________________________________ 

struct  AutomRepos  : public Cv_Repos< AutomElem>
{
    FilterRepos     m_FilterRepos;

    bool        WriteDot( Cv_DotStream &strm)  
    {
        for ( uint32_t i = 1; i < Size(); ++i)
        {
            AutomElem  *si = At( i);
            if (si)
                si->WriteDot( strm); 
        }
        return true;
    }
};

//_____________________________________________________________________________________________________________________________ 
};
