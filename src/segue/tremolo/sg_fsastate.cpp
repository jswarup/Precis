//  sg_fsastate.cpp _____________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/tremolo/sg_fsastate.h" 
#include    "segue/tremolo/sg_dfastate.h" 

using namespace Sg_RExp; 

//_____________________________________________________________________________________________________________________________

bool        FsaRepos::WriteDot( Cv_DotStream &strm)
{
    for ( uint32_t i = 1; i < Size(); ++i)
    {
        Var     si = Get( i);
        if (si)
            si( [this, &strm]( auto k) { k->WriteDot( this, strm); });
    }
    return true;
}

//_____________________________________________________________________________________________________________________________

bool        FsaRepos::DumpDot( const char *path)
{
    std::ofstream           fsaOStrm( path);
    Cv_DotStream			fsaDotStrm( &fsaOStrm, true); 
    return WriteDot( fsaDotStrm);;
}

//_____________________________________________________________________________________________________________________________  

bool  FsaElem::WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm)  
{
    strm << 'R' << GetId() << " [ shape=";

    if ( m_Action)
        strm << "box";
    else
        strm << "ellipse";
    strm << " color=cyan label= <<FONT> N" << GetId() << "<BR />" << RefCount() << "<BR />" ;
    for ( uint32_t i = 0; m_Action && ( i < m_Action->m_Values.size()); ++i)
        strm << " T" << m_Action->m_Values[ i];
    strm << " </FONT>>];\n "; 

    Cv_CArr< FsaId>    dests = Dests(); 
    for ( uint32_t k = 0; k < dests.Size(); ++k)
    {
        FsaElem      *regex = ( FsaElem *) fsaRepos->ToVar( dests[ k]);
        strm << 'R' << GetId() << " -> " << 'R' << regex->GetId() << " [ arrowhead=normal color=black label=<<FONT> ";  
        ChSetFilter     *chSet = fsaRepos->m_FilterRepos.ToVar( m_ChSets[ k]);
        strm << Cv_Aid::XmlEncode( chSet->ToString());
        strm << "</FONT>>] ; \n" ;  
    }
    return true;
}



//_____________________________________________________________________________________________________________________________
 