//  sg_fsastate.cpp _____________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/tremolo/sg_fsastate.h" 
#include    "segue/timbre/sg_distrib.h" 

using namespace Sg_RExp;

//_____________________________________________________________________________________________________________________________

bool        FsaRepos::WriteDot( Cv_DotStream &strm)
{
    for ( uint32_t i = 1; i < Size(); ++i)
    {
        Var     si = Get( i);
        if (si)
            si( [&strm]( auto k) { k->WriteDot( strm); });
    }
    return true;
}


//_____________________________________________________________________________________________________________________________  

bool  FsaElem::WriteDot( Cv_DotStream &strm)  
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
        FsaElem      *regex = ( FsaElem *) dests[ k];
        strm << 'R' << GetId() << " -> " << 'R' << regex->GetId() << " [ arrowhead=normal color=black label=<<FONT> ";  
        strm << Cv_Aid::XmlEncode(  m_ChSets[ k].ToString());
        strm << "</FONT>>] ; \n" ;  
    }
    return true;
}

//_____________________________________________________________________________________________________________________________

std::vector< Sg_ChSet>    FsaSupState::RefineCharDistrib(  void)
{ 
    std::pair< Fs_CharDistrib, int32_t>     validPrtn;

    validPrtn.first.MakeUniversal();

    Fs_CharDistrib::CCLIdImpressor      prtnIntersector(  &validPrtn.first);
 
    Cv_CArr< FsaVar>                    subStates = SubStates();
    for ( uint32_t i = 0; i < subStates.Size(); ++i) 
    {
        FsaClip             state = subStates[ i];
        Cv_CArr< FiltVar>   filters = state.Filters();
/*
        m_SubStates[ i].
        const std::vector< Fs_TransEdge>    &outEdges = ListOutEdges( );

        for ( uint32_t j = 0; j < outEdges.size(); ++j)
        {
            const Fs_TransEdge      &trans = outEdges[ j];
            const Fs_CCL            &ccl = trans.Sifter().CCL();
            aggregator->HfaDfaRepos()->AddCoverCCL( ccl);
            if ( !ccl.IsValid())
            {
                Fs_CCL      eaCCL = trans.Sifter().Clues()->EndAnchorsEAACCL( aggregator->CfgParams()->CCLFlavor());
                if ( eaCCL.IsValid())
                    prtnIntersector->Process( eaCCL, AX_UINT32_MAX);
            }
            else 
                prtnIntersector->Process( ccl, supIdFlg ? AX_UINT32_MAX  : trans.Sifter().Clues()->GetId());
        }
         
        m_SubStates.IntersectOutEdges( &prtnIntersector); 
 
*/  
    }
    Sg_ChSet          validCCL = prtnIntersector.ValidCCL();
     
    prtnIntersector.Over();

/*
    validPrtn.second = validCCL.Negate().GetChar( 0);
    AX_TRACE(( "Domain[ %s] Fail[ 0x%x]",  validPrtn.first.ToString().c_str(), validPrtn.second))
    */
        std::vector< Sg_ChSet>    domain = validPrtn.first.Domain();
        /*
    if ( validPrtn.second != -1)
        domain.erase( domain.begin() +validPrtn.first.Image( validPrtn.second));
*/
    return domain;
}
//_____________________________________________________________________________________________________________________________
 