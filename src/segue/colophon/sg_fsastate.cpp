//  sg_fsastate.cpp _____________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/colophon/sg_fsastate.h" 
#include    "segue/colophon/sg_dfastate.h" 

using namespace Sg_RExp; 

//_____________________________________________________________________________________________________________________________

bool        FsaRepos::WriteDot( Cv_DotStream &strm)
{
    for ( uint32_t i = 1; i < Size(); ++i)
    {
        Var     si = Get( i);
        if (si)
            si( [this, i, si, &strm]( auto k) { k->WriteDot( Cv_CrateId( i, si.GetType()), this, strm); });
    }
    return true;
}
 
//_____________________________________________________________________________________________________________________________  

bool  FsaElem::WriteDot( Id id, FsaRepos *fsaRepos, Cv_DotStream &strm)  
{
    strm << id.GetTypeChar() << id.GetId() << " [ shape=";

    if ( m_Action)
        strm << "box  color=green";
    else
        strm << "ellipse  color=blue";
    strm << " label= <<FONT> " << id.GetTypeChar() << id.GetId() /* << "<BR />" << RefCount() << "<BR/>"*/;
    for ( uint32_t i = 0; m_Action && ( i < m_Action->m_Values.size()); ++i)
        strm << ": " << m_Action->m_Values[ i];
    strm << " </FONT>>];\n"; 

    FsaElemRepos        *elemRepos = static_cast< FsaElemRepos *>( fsaRepos);
    Cv_Seq< FsaId>      dests = Dests(); 
    for ( uint32_t k = 0; k < dests.Size(); ++k)
    {
        Id      regex =  dests[ k];
        strm << id.GetTypeChar() << id.GetId() << " -> " << regex.GetTypeChar() << regex.GetId() << " [ arrowhead=normal color=black label=<<FONT> ";  
        FilterRepos::Var     chVar = elemRepos->m_FilterRepos.ToVar( m_ChSets[ k]);
        strm << Cv_Aid::XmlEncode( elemRepos->m_FilterRepos.ToString( chVar));
        strm << "</FONT>>] ; \n" ;  
    }
    return true;
}



//_____________________________________________________________________________________________________________________________
 