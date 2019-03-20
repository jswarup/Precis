//  sg_fsastate.cpp _____________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/tremolo/sg_fsastate.h" 

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

bool  FsaElem::WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm)  
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

    Cv_CArr< FsaId>    dests = Dests(); 
    for ( uint32_t k = 0; k < dests.Size(); ++k)
    {
        FsaElem      *regex = ( FsaElem *) fsaRepos->ToVar( dests[ k]);
        strm << 'R' << GetId() << " -> " << 'R' << regex->GetId() << " [ arrowhead=normal color=black label=<<FONT> ";  
        strm << Cv_Aid::XmlEncode(  m_ChSets[ k].ToString());
        strm << "</FONT>>] ; \n" ;  
    }
    return true;
}

//_____________________________________________________________________________________________________________________________

Sg_CharDistrib  FsaSupState::RefineCharDistrib( FsaRepos *fsaRepos)
{ 
    Sg_CharDistrib      distrib;

    distrib.MakeUniversal();

    Sg_CharDistrib::CCLIdImpressor      prtnIntersector(  &distrib);
 
    Cv_CArr< FsaId>                    subStates = SubStates();
    for ( uint32_t i = 0; i < subStates.Size(); ++i) 
    {
        FsaClip             state = fsaRepos->ToVar( subStates[ i]);
        Cv_CArr< FiltVar>   filters = state.Filters();
        for ( uint32_t j = 0; j < filters.Size(); ++j)
        {
            ChSetFilter *chSet = filters[ i];
            prtnIntersector.Process( *chSet, CV_UINT32_MAX);
        }
         
    }
    Sg_ChSet          validCCL = prtnIntersector.ValidCCL();
     
    prtnIntersector.Over();
 
    return distrib;
}

//_____________________________________________________________________________________________________________________________

void    FsaSupState::DoConstructTransisition( FsaRepos *fsaRepos)
{
    Sg_CharDistrib          distrib = RefineCharDistrib( fsaRepos);
    std::vector< Sg_ChSet>  domain = distrib.Domain();
    
    uint32_t                sz = domain.size();
    for ( uint32_t i = 0; i < sz; ++i)
    {
        FsaSupState     *supState = new FsaSupState();
    }
/*
    if ( m_Tokens && !m_Tokens->HasTokens())
    {
        delete m_Tokens;
        m_Tokens = NULL;
    }

    if ( m_Tokens)
    { 
        // mark all the shorter match tokens as greedy, as longer match exist
        if ( domain.size() && m_Tokens->GreedyTokens().size())
            m_Tokens->SetGreedyTokenAsShort();
        m_Tokens->SetDepthInfo( Depth());
        if ( IsLoopUp())
            m_Tokens->SetDepthInfo( AX_UINT32_MAX);
        m_Tokens->DoAttach( aggregator, dfaState);
    }
    // ignore the transitions out of this state for swarm && matchall && sc-changes
    if ( !aggregator->CfgParams()->TestTarget( Fs_EngineProp::Txxxx) || !m_Tokens || m_Tokens->GreedyTokens().size() || !m_Tokens->CanChangeSc() )
        DoConstructTransisition( aggregator, dfaState, domain);

    dfaState->SetSaveTH( IsSaveTH());
    if ( aggregator->CfgParams()->TestTarget( Fs_EngineProp::Txxxx) && !aggregator->Context()->ScanFlavor().IsMatchAll())
        dfaState->SetStall( true);
    m_Tokens = NULL;
*/
    return;
}

//_____________________________________________________________________________________________________________________________
 